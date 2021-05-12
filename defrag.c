#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//Global declaration of variables and functions to be utilized
void *func(void *);
int content[111][600000];
int num[111];
int numFiles = 0;
int THREADS = 0;
struct dirent *top_d[111];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

//findFile function to find the mp3 file present in the directories and place the file contents in the correct index in the two dimensional array
void findFile(const char *name, int index){

	DIR *current;
	struct dirent *dirs;
	
	//Open the top level directory present in each thread
	if( !(current = opendir(name)) ){
		return;
	}
	
	//Read from the top level directory to find the required mp3 file
	while( (dirs = readdir(current)) != NULL){
		
		//Only check for directorya dn regular file
		if (dirs->d_type == DT_DIR || dirs->d_type == DT_REG){
			
			//To save the file path name
			char entries[1024];
			
			//Ignore the directories named '.' and '..'
			if(strcmp(dirs->d_name, ".") == 0 || strcmp(dirs->d_name, "..") == 0){
			
				continue;
			}
			
			//Save the file pathe name to the variable entries using snprintf
			snprintf(entries, sizeof(entries), "%s/%s", name, dirs->d_name);
			
			//Only open the regular file with .mp3 extension
			if( dirs->d_name[ strlen(dirs->d_name)-4 ] == '.' && dirs->d_name[ strlen(dirs->d_name)-3 ] == 'm' && dirs->d_name[ strlen(dirs->d_name)-2 ] == 'p' && dirs->d_name[ strlen(dirs->d_name)-1 ] == '3' ) {
				
				char fileN[10];
				
				//Gets the file number of the mp3 file
				for(int k = 0; k < strlen(dirs->d_name)-4; k++){
					fileN[k] = dirs->d_name[k];
				}
				fileN[  strlen(dirs->d_name)-4 ] = '\0';
				
				//Covert the string file number to integer
				int fileNum = atoi(fileN);
				
				//Check for the total number of files extracted
				numFiles++;
				
				//Using fopen to open the mp3 file using the file path name
				FILE *filein = fopen(entries, "r");
				int sample;
				int count;
				
				num[fileNum] = 0;
				while(1){
					
					//Read from the mp3 file 1 byte sample at a time until the end
					count = fread(&sample, 1, 1, filein);
					if (count != 1) {
						break;
					}
					
					//Save 1 byte sample in the two dimensional array named content and thus all the file contents are stored in respective file number
					content[fileNum][num[fileNum]] = sample;
					num[fileNum]++;
    			}
    			
    			//Close the file
    			fclose(filein);
			}
		
			//Recursively find all the files present in the top level directory
			findFile(entries, index+2);
		} else{
			
			//Just do nothing when the file type is neither directory nor regular file
		}
	}
	
	//Close the top level directory
	closedir(current);
}

int main(int argc, char **argv){
	
	struct dirent *entry;
	
	//Return from the program if 3 comman line arguments are not given
	if(argc != 3){
		return -1;
	}
	
	//Open the output file using third argument file name
	FILE *output = fopen(argv[2], "w");
	
	//Change the directory using second argument directory name
	chdir(argv[1]);
	
	//Open the current directory
	DIR *top = opendir(".");
	int top_n = 0;
	while( (entry = readdir(top)) ){
		
		if(entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
			
			//Calculate the number of top level directories to create the same number of threads for program execution
			top_d[top_n] = entry;
			top_n++;
			THREADS++;
		}
	}
	
	//Create the threads for each top level directory
	pthread_t id[THREADS];
	
	for(unsigned long i = 0; i < THREADS; i++){
		int error = pthread_create(&id[i], NULL, func, (void *) i);
		if(error != 0){
			fprintf(stderr, "Could not create thread!!\n");
			printf("No thread\n");
		}
	}
	
	//Join all the threads
	for(int i = 0; i < THREADS; i++){
		pthread_join(id[i], NULL);
	}
	
	//Close the directory that contains everything
	closedir(top);
	
	//Write all the file contents stored in array to the output mp3 file based on ascending order of file number
	for(int j = 0; j < numFiles; j++){
		
		//Since the file was read 1 byte sample at a time, it is written to output file 1 byte at a time.
		//In the output file, all the contents from wach file is written to just one output file and they are written in one output file on the basis of ascending order of file number
		for(int i = 0; i < num[j]; i++){
			fwrite(&content[j][i], 1, 1, output);
		}
	}
		
	//Close the output file
	fclose(output);
	
	return 0;
}

//*func is the pointer function for the creation of threads with arg as argument to be passed
void *func(void *arg){
	
	//Saved the address of each thread
	unsigned long address = (unsigned long) arg;
	
	//Locking the mutex object for each thread
	pthread_mutex_lock(&lock);
	
	//For each thread that contains each top level directory, find all the mp3 files present in that directory
	findFile(top_d[address]->d_name, 0);
	
	//Unlocking the mutex object for each thread
	pthread_mutex_unlock(&lock);
	
	return NULL;
}

/*
	NOTE:
	I believe the threads created are aynchronous because all the threads run at the same time opening each top level directory to find all the other files. Then, after finding mp3 files from all the threads, the file contents are saved in two dimensional array which are shared by all threads.
	Only at the end after joining all the threads, the file contents are written from array to the output file.
	
	Though intended implementation suggests to utilize dynamical global array of file contents, I was unable to get the required result using dynamic memory allocation. 
	I tried to utilize the dynamic memory allocation method. The farthest I reached utilizing dynamic memory allocation was I was able to get the output efficiently with same time span and music. But the problem was some wierd noise was ruining the file in middle. That wierd noise appears only at some certain points. Therefore, I had to give up on trying dynamic memory allocation and just used two dimensional array.
	*/


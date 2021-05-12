defrag: defrag.c
	gcc -std=gnu11 -Wall -Werror -o defrag defrag.c -g -pthread -lm
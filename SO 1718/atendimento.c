#include "head.h"

int msgID;

//named_pipes
int fd;


int main(int argc, char const *argv[])
{

	/* code */
	// open named pipe para escrita
	if ((fd = open(PIPE_NAME, O_WRONLY))<0){
		perror("Erro ao abrir o named pipe para escrever! ");
		exit(0);
	}
	char  word[1024];
	while(1){
		scanf("%s",word);
		write(fd,word,sizeof(word));
	}
	close(fd);
	/*
	msgID = msgget(IPC_PRIVATE, IPC_CREAT|0777);
	if (msgID < 0){
		perror("Erro ao criar message queue");
		exit(0);
	}*/
	return 0;
}
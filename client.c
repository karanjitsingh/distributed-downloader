#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

typedef enum _ClientStatus { WAITING,CONNECTED,TRANSFERING } ClientStatus;

typedef struct _Client {
	ClientStatus status;
} Client;

int main(int argc, char ** argv) {

	Client self;
	self.status = WAITING;

	int status;


	printf("Waiting for broadcast packet...\n");
	

	int sock;
	char buf[25];
	struct sockaddr_in sadd,cadd;
	sock=socket(AF_INET,SOCK_DGRAM,0);
	sadd.sin_family=AF_INET;
	

	sadd.sin_addr.s_addr=inet_addr("0.0.0.0");

	sadd.sin_port=htons(11111);
	int result=bind(sock,(struct sockaddr *)&sadd,sizeof(sadd));
	int len=sizeof(cadd);

	recvfrom(sock,&status,sizeof(int),0,(struct sockaddr *)&cadd,&len);

	printf("Received broadcast packet. Status: %d\n", status);








	int clientSocket;
	int n;
	int pid;
	//struct sockaddr_in sadd;
	
	clientSocket = socket(PF_INET, SOCK_STREAM, 0);
	
	sadd.sin_family = AF_INET;
	sadd.sin_port = htons(13576);
	sadd.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(sadd.sin_zero, '\0', sizeof sadd.sin_zero);	

	len = sizeof sadd;
	while(connect(clientSocket, (struct sockaddr *) &sadd, len));

	int size=100;
	write(clientSocket, &size, sizeof(size));
	
	close(clientSocket);

	return 0;
}
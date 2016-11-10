#include "header.h"

#define CAPACITY 20480

int main(int argc, char ** argv) {

	ClientNode self;
	self.status = WAITING;
	char * host;
	char * url;

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


	//printf("%d.%d.%d.%d\n", (int)(cadd.sin_addr.s_addr&0xFF), (int)((cadd.sin_addr.s_addr&0xFF00)>>8), (int)((cadd.sin_addr.s_addr&0xFF0000)>>16), (int)((cadd.sin_addr.s_addr&0xFF000000)>>24));

	printf("Received broadcast packet. Status: %d\n", status);




	int clientSocket;
	int n;
	int id;
	
	clientSocket = socket(PF_INET, SOCK_STREAM, 0);
	

	sadd.sin_family = AF_INET;
	sadd.sin_port = htons(13576);
	sadd.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(sadd.sin_zero, '\0', sizeof sadd.sin_zero);	

	len = sizeof sadd;
	while(connect(clientSocket, (struct sockaddr *) &sadd, len));

	self.status = CONNECTED;

	int size=CAPACITY;
	int r;
	write(clientSocket, &size, sizeof(size));
	read(clientSocket, &r, sizeof(r));

	read(clientSocket, &size,sizeof(size));
	host = (char *) malloc(size);
	read(clientSocket, host,size);

	read(clientSocket, &size,sizeof(size));
	url = (char *) malloc(size);
	read(clientSocket, url,size);
	

	read(clientSocket, (ClientNode * )&self, sizeof(ClientNode));


	printf("Server return: \n",r);

	printf("ID      : %d\n",self.id);
	printf("Cap     : %d\n",self.cap);
	printf("From    : %d\n",self.bytesFrom);
	printf("Length  : %d\n",self.byteLength);
	printf("Status  : %d\n",self.status);
	printf("Host    : %s\n",host);
	printf("URL     : %s\n",url);

	close(clientSocket);

	self.status = ESTABLISHED;

	return 0;
}
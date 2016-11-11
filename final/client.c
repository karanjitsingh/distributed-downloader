#include "header.h"

#define CAPACITY 3500000

char FILENAME[100];

void downloadFile(char * host, char * message_fmt) {
	int portno = 80;

	struct hostent *server;
	struct sockaddr_in serv_addr;
	int sockfd, bytes, sent, received, total;
	char message[1024];//,response[4096];

	char * response;
	response = (char *)malloc(4096 * sizeof(char));
	memset(response,'x',4096 * sizeof(char));

	sprintf(message,message_fmt);
	printf("Request:\n%s\n",message);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) error("ERROR opening socket");

	server = gethostbyname(host);
	if (server == NULL) error("ERROR, no such host");

	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
		error("ERROR connecting");


	printf("Message:\n%s\n",message);

	total = strlen(message);
	sent = 0;


	do {
		bytes = write(sockfd,message+sent,total-sent);
		if (bytes < 0)
			error("ERROR writing message to socket");
		if (bytes == 0)
			break;
		sent+=bytes;
	} while (sent < total);

	/* receive the response */
	memset(response,0,4096 * sizeof(char));
	
	received = 0;
	FILE *file;
	file = fopen(FILENAME,"wb");  // w for write, b for binary
	

	
	int x=4;

	bytes = read(sockfd,response+x,4);

	while(1) {
		bytes = read(sockfd,response+x,1);
		response[x+1]='\0';
		printf("%s", response+x);
		if (bytes < 0){
			error("ERROR reading response from socket");
			return;
		}
		if (bytes == 0)
			return;


		if(x>=3) {
			response[x+1] = '\0';
			//printf("%s", response + x - 3);
			if(strcmp(response + x-3,"\r\n\r\n") == 0){
				break;
			}
		}

		x++;


	}

	printf("Now reading.\n");

	while(1) {
		bytes = read(sockfd,response,sizeof(response));
		if (bytes < 0)
			error("ERROR reading response from socket");
		if (bytes == 0)
			break;

		//printf("%s",response);
		fwrite(response,bytes,1,file);
	}

	fclose(file);


	close(sockfd);
}

int main(int argc, char ** argv) {


	srand(time(NULL));
	sprintf(FILENAME,"./bin/file%d.bin",rand()%1000);




	ClientNode self;
	self.status = WAITING;
	char * host;
	char * url;

	int status=12;


	printf("Waiting for broadcast packet...\n");
	

	int sock;
	char buf[25];
	struct sockaddr_in sadd,cadd;
	sock=socket(AF_INET,SOCK_DGRAM,0);

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
	struct timeval tv;
	tv.tv_sec = 5;  /* 30 Secs Timeout */
	tv.tv_usec = 0;  // Not init'ing this can cause strange errors
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));	



	sadd.sin_family=AF_INET;
	sadd.sin_addr.s_addr=inet_addr("0.0.0.0");
	sadd.sin_port=htons(11111);


	int result=bind(sock,(struct sockaddr *)&sadd,sizeof(sadd));
	int len=sizeof(cadd);

	while(status!=1)
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
	

start:
	read(clientSocket, (ClientNode * )&self, sizeof(ClientNode));

	if(self.byteLength == 0)
		self.byteLength = CAPACITY;

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




	char message_fmt[3000];
	sprintf(message_fmt,"GET %s HTTP/1.0\r\nRange: bytes=%d-%d\r\n\r\n",url,self.bytesFrom,self.bytesFrom+self.byteLength-1);

	printf("Message:\n%s\n", message_fmt);

	downloadFile(host,message_fmt);





	printf("Sending back to server.\n");

	//Send data
	clientSocket = socket(PF_INET, SOCK_STREAM, 0);
	while(connect(clientSocket, (struct sockaddr *) &sadd, len));
	

	write(clientSocket, &self, sizeof(ClientNode));


	FILE *file;
	unsigned char response[1000];  // array of bytes, not pointers-to-bytes
	size_t bytesRead = 0;

	file = fopen(FILENAME, "rb");   

	if (file != NULL)    
	{
		while ((bytesRead = fread(response, 1, sizeof(response), file)) > 0){
			write(clientSocket,response,bytesRead);
		}
	}

	fclose(file);

	int closing;

	read(clientSocket,&closing,sizeof(closing));

	if(!closing) {
		goto start;
	}

	close(clientSocket);





	return 0;
}
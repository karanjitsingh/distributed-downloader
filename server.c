#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

typedef enum _ClientStatus { WAITING,CONNECTED,TRANSFERING } ClientStatus;

typedef struct _ClientNode {
	struct sockaddr_in cadd;
	int id;
	ClientStatus status;
} ClientNode;

typedef struct _FileInfo {
	long long length;
} FileInfo;

void error(const char *msg) { perror(msg); exit(0); }

FileInfo * generateFileInfo(char * response, int size) {

	char * curLine = response;
	FileInfo *file;

	while(curLine)
	{
		char * nextLine = strchr(curLine, '\n');
		if (nextLine) *nextLine = '\0';
			
		if(strstr(curLine,"Content-Length:")) {
			file = (FileInfo *) malloc(sizeof(FileInfo));
			file->length = atoll(curLine + strlen("Content-Length:"));
		}

		if (nextLine) *nextLine = '\n';    
			curLine = nextLine ? (nextLine+1) : NULL;
	}
	return file;
}

FileInfo * requestFile(char * host, char * message_fmt) {
	int portno =		80;

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
	total = 4096 * sizeof(char)-1;
	received = 0;
	do {
		bytes = read(sockfd,response+received,total-received);
		if (bytes < 0)
			error("ERROR reading response from socket");
		if (bytes == 0)
			break;
		received+=bytes;
	} while (received < total);

	if (received == total)
		error("ERROR storing complete response from socket");

	close(sockfd);

	printf("Response:\n%s\n",response);

	return generateFileInfo(response,4096*sizeof(char));
}

int main(int argc, char ** argv) {

	char *host =		"localhost";
	char *message_fmt = "HEAD /video.mp4 HTTP/1.0\r\n\r\n";

	//Get file
	FileInfo * file = requestFile(host,message_fmt);
	printf("File length:%lld\n", file->length);


	//Run prober as fork
	int proberPID;
	if(proberPID=fork()==0) {
		execvp("./prober",NULL);
		exit(0);
	}





	int defSock, newSock, len;
	char * buffer;
	struct sockaddr_in sadd, cadd;

	socklen_t addr_size;
	defSock = socket(PF_INET, SOCK_STREAM, 0);
	
	sadd.sin_family = AF_INET;
	sadd.sin_port = htons(12345);
	sadd.sin_addr.s_addr = INADDR_ANY;

	bind(defSock, (struct sockaddr *) &sadd, sizeof(sadd));

	if(listen(defSock,5)==0)
		printf("Listening\n");
	else
		printf("Error\n");

	len = sizeof(cadd);
	newSock = accept(defSock, (struct sockaddr *) &cadd, &len);
	
	int size;
	read(newSock,&size,sizeof(size));

	//write(newSock,out,size);


	close(newSock);


	return 0;
}
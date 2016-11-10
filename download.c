#include "header.h"

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
	file = fopen("video.mp4","wb");  // w for write, b for binary
	

	
	int x=0;

	while(x<300) {
		bytes = read(sockfd,response+x,1);
		//response[4]='\0';
		if (bytes < 0){
			error("ERROR reading response from socket");
			return;
		}
		if (bytes == 0)
			return;

		//printf("%s", response);

		if(x>=3) {
			response[x-3+1] = '\0';
			printf("%s", response + x - 3);
			//if(strcmp(response + x-3,"\r\n\r\n") == 0){
			//	break;
			//}
		}

		x++;


	}
	return;

	printf("Now reading...");

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

int main() {
	
	char *host =		"localhost";
	char message_fmt[2084];
	char *url = "/video.mp4";


	sprintf(message_fmt,"GET %s HTTP/1.0\r\n\r\n",url,1,4);


	downloadFile(host,message_fmt);

	int i;

}
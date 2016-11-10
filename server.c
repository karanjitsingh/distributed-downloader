#include "header.h"

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

void probeNodes() {
	//printf("Probing nodes...\n");


	int sock;
	struct sockaddr_in address;
	sock=socket(AF_INET,SOCK_DGRAM,0);
	address.sin_family=AF_INET;
	address.sin_addr.s_addr=inet_addr("255.255.255.255");


	int broadcastEnable=1;
	int ret=setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));


	address.sin_port=htons(11111);

	int status=1;

	int len=sizeof(address);
	

	//printf("Sending broadcast packet...\n");
	sendto(sock,&status,sizeof(int),0,(struct sockaddr *)&address,len);

}

int main(int argc, char ** argv) {
	

	char *host =		"localhost";
	char *message_fmt = "HEAD /video.mp4 HTTP/1.0\r\n\r\n";
	char *url = "/video.mp4";

	int i;

	//Get file
	FileInfo * file = requestFile(host,message_fmt);
	printf("File length:%lld\n", file->length);


	//Run prober as fork
	/*int proberPID;
	if(proberPID=fork()==0) {
		while(1) {
			probeNodes();
			sleep(3);
		}
		exit(0);
	}*/


	ClientNode ** connectedNodes;
	ClientNode * node;


	int defSock, newSock, len;
	char * buffer;
	struct sockaddr_in sadd, cadd;
	socklen_t addr_size;

	defSock = socket(PF_INET, SOCK_STREAM, 0);



	//Set socket options
	setsockopt(defSock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
	struct timeval tv;
	tv.tv_sec = 5;  /* 30 Secs Timeout */
	tv.tv_usec = 0;  // Not init'ing this can cause strange errors
	setsockopt(defSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));	


	sadd.sin_family = AF_INET;
	sadd.sin_port = htons(13576);
	sadd.sin_addr.s_addr = inet_addr("127.0.0.1");



	len = sizeof(cadd);


	srand(time(NULL));
	int nodeCount=0;


	//while(1) {

		bind(defSock, (struct sockaddr *) &sadd, sizeof(sadd));

		if(listen(defSock,SOMAXCONN)==0)
			printf("Listening\n");
		else
			printf("Error\n");

		newSock = accept(defSock, (struct sockaddr *) &cadd, &len);

		if(newSock == -1) {
			printf("Cannot find more nodes. %d\n",nodeCount);
			//break;
		}

		nodeCount++;

		if(nodeCount ==1)
			connectedNodes = (ClientNode **)malloc(nodeCount * sizeof(ClientNode *));
		else
			connectedNodes = (ClientNode **)realloc(connectedNodes,nodeCount * sizeof(ClientNode *));

		node = (ClientNode *) malloc(sizeof(ClientNode));
		connectedNodes[nodeCount-1] = node;


		int cap;
		read(newSock,&cap,sizeof(cap));
		write(newSock, &nodeCount, sizeof(nodeCount));

		printf("New node with cap %d\n",cap);

		node->cap = cap;
		node->status = ESTABLISHED;
		node->id=nodeCount;
		node->socket = newSock;


		int size = strlen(host);
		write(newSock, &size, sizeof(size));
		write(newSock, host, size);

		size = strlen(url);
		write(newSock, &size, sizeof(size));
		write(newSock, url, size);

			

		//close(newSock);
	//}
	

	int total=0;
	int byteTotal = 10;
	int current=-1;

	for(i=0;i<nodeCount;i++) {
		total += connectedNodes[i]->cap;

	}


	//Sort nodes here

	if(byteTotal < total) {

		for(i=0;i<nodeCount;i++) {

			current++;
			connectedNodes[i]->bytesFrom = current;
			if(byteTotal-current>=connectedNodes[i]->cap){
				connectedNodes[i]->byteLength = connectedNodes[i]->cap;
				current += connectedNodes[i]->cap;
			}
			else{
				connectedNodes[i]->byteLength = byteTotal-current;
				current = byteTotal;
			}


			printf("ID      : %d\n",connectedNodes[i]->id);
			printf("Cap     : %d\n",connectedNodes[i]->cap);
			printf("From    : %d\n",connectedNodes[i]->bytesFrom);
			printf("Length  : %d\n",connectedNodes[i]->byteLength);
			printf("Status  : %d\n",connectedNodes[i]->status);


			write(connectedNodes[i]->socket, node, sizeof(ClientNode));

			close(connectedNodes[i]->socket);

		}


	}
	else {
		//while(current < byteTotal)

	}





	
	

	return 0;
}
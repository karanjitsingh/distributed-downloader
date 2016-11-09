#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */

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

int main() {
	while(1) {
		probeNodes();
		sleep(3);
	}
}
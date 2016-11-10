#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef enum _ClientStatus { WAITING,ESTABLISHED,CONNECTED,TRANSFERING,LOST } ClientStatus;

typedef struct _ClientNode {
	int cap;
	int id;
	ClientStatus status;
	int pid;
	int socket;
} ClientNode;

typedef struct _FileInfo {
	long long length;
} FileInfo;

void error(const char *msg) { perror(msg); exit(0); }
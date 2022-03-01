

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>
#include <netinet/in.h>



/*
void * doit(void *arg);
void process_request(int arg);
int tcp_listen(char* a1, char *a2, socklen_t* len);


int main(int argc, char **argv) {
	int listenfd, connfd;
	pthread_t tid;
	socklen_t clilen, addrlen;
	struct sockaddr *cliaddr;

	// Contiene las llamadas a socket(), bind() y listen() ///
	listenfd = tcp_listen(argv[1], argv[2], &addrlen);
	for ( ; ; ) {
		connfd = accept(listenfd, cliaddr, &clilen);
		pthread_create(&tid, NULL, &doit, (void *) connfd);
	}

}

void * doit(void *arg) {
	void web_child(int);

	pthread_detach(pthread_self()); // se desengancha
	process_request((int) arg);

	close((int) arg);
	return(NULL);
}



int tcp_listen(char* a1, char *a2, socklen_t* len){
	int lis_res, bind_res, sockfd;
	struct sockaddr_in sadr;
	
	sadr.sin_family = AF_INET;
	sadr.sin_addr.s_addr = INADDR_ANY;
	sadr.sin_port = htons(8080);
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);//int, int, int -> int///
	bind_res = bind(sockfd, (struct sockaddr *) &sadr, *len);//int, const struct sockaddr, socklen_t -> int///
	lis_res = listen(sockfd, 3);// -> int///
	
	return lis_res;
}
*/


#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h> /*Para inet_ntop*/
#include "tcp.h"
#include "types.h"
#define MAX 80
#define PORT 8080
#define SA struct sockaddr


void bzero(void* buff, int size){
	int i;
	char *buffer=buff;
	for(i=0; i<size; i++){
		buffer[i]='\0';
	}
}


/*/ Function designed for chat between client and server./*/
int func(int connfd)
{
    char buff[MAX];
    int n;
    /*/ infinite loop for chat/*/
    for (;;) {
        bzero(buff, MAX);
   
        /*/ read the message from client and copy it in buffer/*/
        read(connfd, buff, sizeof(buff));
        /*/ print buffer which contains the client contents/*/
        printf("From client: %s\t To client : ", buff);
        bzero(buff, MAX);
        n = 0;
        /*/ copy server message in the buffer/*/
        while ((buff[n++] = getchar()) != '\n')
            ;
   
        /*/ and send that buffer to client/*/
        write(connfd, buff, sizeof(buff));
   
        /*/ if msg contains "Exit" then server exit and chat ended./*/
        if (strncmp("exit", buff, 4) == 0) {
            printf("Server Disconnect Client\n");
            return 0;
        }
        if (strncmp("close", buff, 5) == 0) {
            printf("Server Close\n");
            return 1;
        }
    }

}

void process_request(int arg){

    if(arg < 0); return NULL; /*Error porque lo que pasa el cliente esta mal*/

    /*En arg, encontrar la forma de identificar la orden*/
    /*Entonces añadir if y else para llamar a otras funciones dependiendo de lo anterior*/

	printf("Request %d\n", arg);

	return;
}
   
/*/ Driver function/*/
int main()
{
    int sockfd, connfd;
    struct sockaddr_in servaddr;
    char ip[50];
   
    sockfd = tcp_listen();
    if(sockfd < 0){
        if(DEBUG) printf("server main > tcp_listen error\n");
        return 1;
    }


    inet_ntop(AF_INET, &(servaddr.sin_addr), ip, INET_ADDRSTRLEN);
    printf("Server IP = %s\n", ip);

    while(TRUE){

        /*/ Accept the data packet from client and verification/*/
        connfd = tcp_accept(sockfd);
        if(connfd < 0){
            if(DEBUG) printf("server main > tcp_accept error\n");
            
            close(sockfd);
            return 2;
        }

        process_request(connfd);

    
        /*/ Function for chatting between client and server/*/
        if(func(connfd)){
            
            if(DEBUG) printf("server main > Server Closed\n");
            break;
        }
    }
   
    /*/ After chatting close the socket/*/
    close(sockfd);
	return 0;
}

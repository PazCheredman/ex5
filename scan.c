#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>         
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>


#define MAX_PORTS 200
#define TIMEOUT 100
extern char * optarg;

static int connect_with_timeout(int soc, struct sockaddr *srvAddr, int size, int timeout)
{
	struct timeval tv;
	fd_set myset;
	int arg;
	int res;
	struct sockaddr_in *srv = (struct sockaddr_in *)srvAddr;
        
	/*  Set non-blocking */ 
	arg = fcntl(soc, F_GETFL, NULL); 
	arg |= O_NONBLOCK; 
	fcntl(soc, F_SETFL, arg);
	
	res = connect(soc, srvAddr, size); 
   	 if (res < 0){ 
	   if (errno == EINPROGRESS){ 
	    tv.tv_sec = timeout / 1000; //in milli seconds
	    tv.tv_usec = (timeout - tv.tv_sec *1000) * 1000; //in micro seconds
	    FD_ZERO(&myset); 
	    FD_SET(soc, &myset); 
	    
		if (select(soc+1, NULL, &myset,NULL, &tv) > 0) {
	     		int lon = sizeof(int); 
	     		int valopt;
	     		getsockopt(soc, SOL_SOCKET, SO_ERROR, (void*)(&valopt), (socklen_t *)&lon);
	    	 	if (valopt){
	         		printf("%d:Error in connection() %d - %s\n", soc,valopt, strerror(valopt));
	         		goto exit_error;
	     		}
	    	}
	    	else{
	        	printf("%d:Connection Timeout [%d], port %d\n", soc,timeout,ntohs(srv->sin_port));
	        	goto exit_error;
	    	}
	   }
	else {
	     printf("%d:Error connecting %d - %s\n",soc, errno, strerror(errno));
	     goto exit_error;
	 }
      }
	/* success Set to blocking mode again... */ 
		 arg = fcntl(soc, F_GETFL, NULL); 
	 	arg &= (~O_NONBLOCK); 
		 fcntl(soc, F_SETFL, arg); 
		 return soc;
	exit_error: //label declaring
    		return -1;
}


/* connect to ip socket on server */
int scan_ports(char *ipAddr, int timeout,int rnd){

	/* client sock should be context dependent */
	struct sockaddr_in srvAddr; /* Server address */
	int client_sock;
	int res;
        int port;
	/* Construct remote address structure */
	memset(&srvAddr, 0, sizeof(srvAddr));  	/* Zero out structure */
	srvAddr.sin_family = AF_INET;              	  /* Internet address family */
	srvAddr.sin_addr.s_addr = inet_addr(ipAddr); /* Remote address*/

	for(int i=1; i<MAX_PORTS; i++){
	      	 client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	        if(client_sock == -1){
        	        printf("cannot open socket %m\n"); //%m means error message
       		 }	
 
	if (rnd) {
         port = rand() % (MAX_PORTS-1) + 1;
        } else {
        port = i;
       }
        srvAddr.sin_port = htons(port);   /* Remote port */
	res = connect_with_timeout(client_sock, (struct sockaddr *)&srvAddr, sizeof(srvAddr),timeout);

		if(res >0){
			printf("port =%d \n", port);
		}
	close(client_sock);
        }
	return 0;
}


int main(int argc, char* argv []){ //argc-how many parameters received , argv[i] is the parameter 

	int rnd = 0;
        int timeout=TIMEOUT;
	int opt; 
        char ip[16];
        while ((opt = getopt(argc, argv, "res:i:")) != -1) {
               switch (opt) {
               case 'r':
                   rnd = 1;
                   break;
               case 's':
                   timeout = atoi(optarg);
                   break;
               case 'i':
                 strcpy(ip,optarg);
                 break;
               default: /* '?' */
                   printf("Usage: %s [-s msecs] [-r] name\n",
                           argv[0]);
                   return 0;
               }
           }


		printf("checking for %s \n", ip);
		scan_ports(ip, timeout,rnd);

return 0;
}

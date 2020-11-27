#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h> 
#include <ifaddrs.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#define port 8888

void main(int argc, char** argv)
{
	struct hostent *host;
	int err, i , sock;
	struct sockaddr_in sa;
	char *hostname = argv[1];
	char name[32];
	//Initialise the sockaddr_in structure
	strncpy((char*)&sa , "" , sizeof sa);
	sa.sin_family = AF_INET;
	
	//direct ip address, use it
	if(isdigit(hostname[0]))
	{
		
		sa.sin_addr.s_addr = inet_addr(hostname);
		
	}
	//Resolve hostname to ip address
	else if( (host = gethostbyname(hostname)) != 0)
	{
		
		strncpy((char*)&sa.sin_addr , (char*)host->h_addr , sizeof sa.sin_addr);
		
	}
	else
	{
		herror(hostname);
		exit(2);
	}
	
	//Start the port scan loop
	i = port;
		//Fill in the port number
		sa.sin_port = htons(i);
		//Create a socket of type internet
		sock = socket(AF_INET , SOCK_STREAM , 0);
		
		//Check whether socket created fine or not
		if(sock < 0) 
		{
			perror("\nSocket");
			exit(1);
		}
		//Connect using that socket and sockaddr structure
		err = connect(sock , (struct sockaddr*)&sa , sizeof sa);
	//printf("notify thread: %s",hostname);
		//not connected
		
		if( err < 0 )
		{
			//printf("%s %-5d %s\r" , hostname , i, strerror(errno));
			fflush(stdout);
		}
		//connected
		else
		{
			printf("%-5d open at ip--%s\n",  i,hostname);
			read(sock,name,32);
			printf("name:%s\n",name);
		}
		close(sock);
	
	printf("\r");
	fflush(stdout);
	return;
}

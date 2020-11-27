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
#define port 1200


void call(char* ip)
{
	system(ip);
}

int msleep(unsigned int tms) {
  return usleep(tms * 1000);
}

int main()
{
	int i =0,j=0;
	char ips[10];
	char ip[20];
	char temp[10];
	unsigned char ip_address[15];
	int fd;
	struct ifreq ifr;
	fd = socket(AF_INET,SOCK_DGRAM,0);
	ifr.ifr_addr.sa_family = AF_INET;
	memcpy(ifr.ifr_name,"wlx502b73c48d3b",IFNAMSIZ-1);
	ioctl(fd,SIOCGIFADDR,&ifr);
	close(fd);
	strcpy(ip_address,inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	strncpy(ips,ip_address,10);
	pthread_t application;	
		for(j=0;j<255;j++)
			{
				strcpy(ip,ips);
				sprintf(temp, ".%d",j);
				strcat(ip,temp);
				char a[100] = "./check ";
				strcat(a,ips);
				if(strcmp(ips,ip_address)==0)
					continue;
				pthread_create( &application , NULL , call , a);
				msleep(60);
			}
			//pthread_join( application , NULL);
			sleep(4);

return 1;
}



#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>    
#include <sys/un.h>
#include <pthread.h> 
#include <netinet/in.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <termios.h>

#define SER_PORT 8888
#define MAX_CLIENTS 100
#define BUFFER_SZ 2048
#define LENGTH 2048

static struct termios old, new;

void gotoxy(int x,int y)    
{
    printf("%c[%d;%df",0x1B,y,x);
}

/* Initialize new terminal i/o settings */
void initTermios(int echo) 
{
  tcgetattr(0, &old); //grab old terminal i/o settings
  new = old; //make new settings same as old settings
  new.c_lflag &= ~ICANON; //disable buffered i/o
  new.c_lflag &= echo ? ECHO : ~ECHO; //set echo mode
  tcsetattr(0, TCSANOW, &new); //apply terminal io settings
}

/* Restore old terminal i/o settings */
void resetTermios(void) 
{
  tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo) 
{
  char ch;
  initTermios(echo);
  ch = getchar();
  resetTermios();
  return ch;
}

/* 
Read 1 character without echo 
getch() function definition.
*/
char getch(void) 
{
  return getch_(0);
}

/* 
Read 1 character with echo 
getche() function definition.
*/
char getche(void) 
{
  return getch_(1);
}
void green () {
  printf("\033[1;32m");
}
void blue(){
printf("\033[1;36m");
}

void yellow (){
  printf("\033[01;33m");
}

void reset () {
  printf("\033[0m");
}

int msgcount = 0;
static _Atomic unsigned int cli_count = 0;
static int uid = 10;
volatile sig_atomic_t flag = 0,wait = 1,pflag = 0;
int sockfd = 0;
char name[32];
struct Qnode
{
	char message[1000];
	struct Qnode *next;
};

struct Queue
{
	struct Qnode *front,*rear;
};

typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
} client_t;

client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;



void str_trim_lf (char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) { 
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}

void cntrlc(int sig) {
    flag = 1;
    wait = 1;
    pthread_detach(pthread_self());
}

void send_msg_handler() {
  	char message[LENGTH] = {};
	char buffer[LENGTH + 32] = {},a;
	int i=0;	
  	
  while(1) {
    msgpointer();
    fgets(message, LENGTH, stdin);
    fputs("\033[A\033[2K",stdout);
    rewind(stdout);
    /*while(1)
        {
                if(i%30 == 0 && i/30!=0)
                	printf("\n");
                a = getc(stdin);
                if(a=='\n')
		{
			message[i] = '\0';
			break;
		}
                message[i++] = a;
        }*/
    print_send_messages(message);
    str_trim_lf(message, LENGTH);
    msgcount++;
    
    if (strcmp(message, "exit") == 0) {
			break;
    } else {
      sprintf(buffer, "%s: %s\n", name, message);
      send(sockfd, buffer, strlen(buffer), 0);
    }

		bzero(message, LENGTH);
    bzero(buffer, LENGTH + 32);
  }
  cntrlc(2);
}

void print_send_messages(char *msg)
{
	int i=0;
	while(msg[i] != '\0')
	{
		printf("%c",msg[i++]);
		if(i%30==0 && i/30!=0)
			printf("\n");
						
	}
	printf("\n");
}

void print_recv_messages(char *msg)
{
	int i=0;
	printf("\t\t\t\t\t\t\t");
	while(msg[i] != '\0')
	{
		printf("%c",msg[i++]);
		if(i%30==0 && i/30!=0)
			printf("\n\t\t\t\t\t\t\t");
						
	}
	printf("\n");
}

void recv_msg_handler() {
	char message[LENGTH] = {};
	struct Queue* queuerec = (struct Queue*)malloc(sizeof(struct Queue));
	queuerec->front = queuerec->rear = NULL;
	const char s[2] = ":";
	char *t1,*t2;
	
  while (1) {
		int receive = recv(sockfd, message, LENGTH, 0);
    if (receive > 0) {
      //printf("%s", message);
      print_recv_messages(message);
      msgpointer();
      enQueue(queuerec,message);
      t1 = strtok(message, s);
      t2 = strtok(NULL, s);
      //if(t2!=NULL)
      //{
      deQueue(queuerec,t1);
      //}
    } else if (receive == 0) {
			break;
    } else {
			// -1
		}
		memset(message, 0, sizeof(message));
  }
}


void msgpointer() {
    printf("\r%s", "> ");
    fflush(stdout);
}



void print_client_addr(struct sockaddr_in addr){
    printf("%d.%d.%d.%d",
        addr.sin_addr.s_addr & 0xff,
        (addr.sin_addr.s_addr & 0xff00) >> 8,
        (addr.sin_addr.s_addr & 0xff0000) >> 16,
        (addr.sin_addr.s_addr & 0xff000000) >> 24);
}

void queue_add(client_t *cl){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i){
		if(!clients[i]){
			clients[i] = cl;
			break;
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}


void queue_remove(int uid){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid == uid){
				clients[i] = NULL;
				break;
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}


void send_message(char *s, int uid){
	pthread_mutex_lock(&clients_mutex);
	struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
	queue->front = queue->rear = NULL;	
	for(int i=0; i<MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid != uid){
				if(write(clients[i]->sockfd, s, strlen(s)) < 0){
					perror("ERROR: write to descriptor failed");
					break;
				}
				else{
				enQueue(queue,s);
				deQueue(queue,clients[i]->name);
				}	
							
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}


void *handle_client(void *arg){
	char buff_out[BUFFER_SZ];
	char name[32];
	int leave_flag = 0;

	cli_count++;
	client_t *cli = (client_t *)arg;
	
	if(recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1){
		printf("Didn't enter the name.\n");
		leave_flag = 1;
	} else{
		strcpy(cli->name, name);
		sprintf(buff_out, "%s has joined\n", cli->name);
		printf("%s", buff_out);
		printf("\n----------------------------------------------------------------------------------------------------------------------------------------------------------------\n\n");
		send_message(buff_out, cli->uid);
	}

	bzero(buff_out, BUFFER_SZ);

	while(1){
		if (leave_flag) {
			break;
		}

		int receive = recv(cli->sockfd, buff_out, BUFFER_SZ, 0);
		if (receive > 0){
			if(strlen(buff_out) > 0){
				send_message(buff_out, cli->uid);

				str_trim_lf(buff_out, strlen(buff_out));
				
			}
		} else if (receive == 0 || strcmp(buff_out, "exit") == 0){
			sprintf(buff_out, "%s:%s has left\n", cli->name,cli->name);
			printf("%s", buff_out);
			send_message(buff_out, cli->uid);
			leave_flag = 1;
			pthread_detach(pthread_self());
		} else {
			printf("ERROR: -1\n");
			leave_flag = 1;
		}

		bzero(buff_out, BUFFER_SZ);
	}

  close(cli->sockfd);
  queue_remove(cli->uid);
  free(cli);
  cli_count--;
  wait--;
  pthread_detach(pthread_self());

	return NULL;
}

void connectcontact(char *ip)
{
	
	int port = SER_PORT;

	signal(SIGINT, cntrlc);

	struct sockaddr_in server_addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip);
  server_addr.sin_port = htons(port);


  
  int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (err == -1) {
		printf("ERROR: connect\n");
		onlinechat();
		return;
	}
	
	send(sockfd, name, 32, 0);

	pthread_t send_msg_thread;
  if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
    return;
	}

	pthread_t recv_msg_thread;
  if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
		return;
	}

	while (1){
		if(flag){
			printf("\nBye\n");
			close(sockfd);
			wait --;
			menu();
			break;
    }
	}

	return;
}

int server()
{
	int port = SER_PORT;
	int option = 1;
	int connfd = 0;
	int listenfd = 0;
  struct sockaddr_in serv_addr;
  struct sockaddr_in cli_addr;
  pthread_t tid;


  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);


	signal(SIGPIPE, SIG_IGN);

	if(setsockopt(listenfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0){
		perror("ERROR: setsockopt failed");
    return;
	}

  if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("ERROR: Socket binding failed");
    return;
  }


  if (listen(listenfd, MAX_CLIENTS) < 0) {
    perror("ERROR: Socket listening failed");
    return;
	}

	while(1){
		socklen_t clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);
		wait ++;
		if(pflag>0)
			{
			while (pflag>0)
			{}
			}
		if(wait == 2)
		{	
			pthread_t connserver;
			pthread_create( &connserver , NULL ,  connectcontact , "127.0.0.1");
		}

		if((cli_count + 1) == MAX_CLIENTS){
			printf("Max clients reached. Rejected: ");
			print_client_addr(cli_addr);
			printf(":%d\n", cli_addr.sin_port);
			close(connfd);
			continue;
		}

		client_t *cli = (client_t *)malloc(sizeof(client_t));
		cli->address = cli_addr;
		cli->sockfd = connfd;
		cli->uid = uid++;

		queue_add(cli);
		pthread_create(&tid, NULL, &handle_client, (void*)cli);

	}

	return;
}




void enQueue(struct Queue* q, char m[1000])
{
	if (q->rear == NULL)
	{
		q->rear = (struct Qnode *)malloc(1*sizeof(struct Qnode));
		q->rear->next = NULL;
		strcpy(q->rear->message,m);
		q->front = q->rear;
	}
	else
	{
		struct Qnode *temp=(struct Qnode *)malloc(1*sizeof(struct Qnode));
		q->rear->next = temp;
		strcpy(temp->message,m);
		temp->next = NULL;
		q->rear = temp;
	}
}

void deQueue(struct Queue *q,char *file)
{
	FILE *f;
	const char t[2] = ":";
	char *t1,*t2;
	t1 = strtok(q->front->message, t);
      	t2 = strtok(NULL, t);
      	if(!(f = fopen(file,"r")))
      	{
      		f=fopen(file,"w");
		fprintf(f,"Contact Name: %s\n Messages:\n",name);
		fclose(f);
		f=fopen("contactlist.txt","a");
		fprintf(f,"%s\n",file);
		fclose(f);
      	}
	f=fopen(file,"a+");
	time_t currentTime;
	time(&currentTime);
	
	if(t2!=NULL)
		fprintf(f,"%s: %s<%s>\n\n",ctime(&currentTime),t2,t1);
	else
		fprintf(f,"%s: %s<%s>\n\n",ctime(&currentTime),q->front->message,file);
	
	fclose(f);

	struct Qnode *temp = q->front;
	q->front = q->front->next;
	if (q->front == NULL)
		q->rear = NULL;
	free(temp);
}


void login()
{
	char pwd[8],pin[8],pswd[8],a;
	int i=0;
	printf("Enter the 4 digit pin to get into the community: ");
	while(1)
        {
                a = getch();
                printf("*");
                if(a=='\n')
		{
			pwd[i] = '\0';
			break;
		}
                pwd[i++] = a;
        }
	
	FILE *x;
	x=fopen("pin.txt","r");
	fseek(x,-4,SEEK_END); 	
	fgets(pin,5,x);
	fclose(x);
	
	if((atoi(pwd)) == atoi(pin))
		{	green();
			printf("\nLogged in...");
			reset();
			pthread_t thread_id;
			pthread_create( &thread_id , NULL ,  server , NULL);
			if(wait>1)
			{
			while (wait>1)
			{}
			}
			
			menu();
			pthread_join( thread_id , NULL);
		}
	else
		{
		printf("Wrong pin...");
		login();
		return;
		}
	
	}

void pcontacts()
{		system("clear");
		int size,customeradd;
		char string[100];
		FILE *f;
		f=fopen("contactlist.txt","r");
		fseek(f,0,SEEK_END);
		size=ftell(f);
		
		if(size==0)
		{
			printf("\nNo contacts to display\n");
			exit(0);
		}
		
		else
		{
			fseek(f,0,SEEK_SET);
			while(fgets(string,100,f)!=NULL)
				printf("%s",string);
			fclose(f);
		}
		if(wait>1)
		{
			while (wait>1)
			{
			
			}
		}
		printf("\n\n\n\n\n");
		
		chat();
}

void addcontact()
{		system("clear");
		char n[10],name[100];
		
		if(wait>1)
		{
			while (wait>1)
			{
			
			}
		}
		
		printf("\nEnter the name of the contact: ");
		scanf("%s", name);
					
		FILE *f;
					
		f=fopen(name,"w");
		fprintf(f,"Contact Name: %s\n Messages:\n",name);
		fclose(f);
		f=fopen("contactlist.txt","a");
		fprintf(f,"%s\n",name);
		fclose(f);
		printf("\nContact name successfully added %s \n\n\\n\n\n",name);
		menu();
}

void chat()
{
	char person[100],string[100];
	if(wait>1)
		{
			while (wait>1)
			{
			
			}
		}
	
	printf("\n Enter the contact name to chat with:");
	scanf("%s",person);
	if(exists(person))
	{
		FILE *f;
		f=fopen(person,"r");
		fseek(f,0,SEEK_SET);
			while(fgets(string,100,f)!=NULL)
				printf("%s",string);
			onlinechat(person);
			fclose(f);
	}
	else
	{
		printf("\n no such contact...\n");
		chat();
		return;
	}
	
}

void onlinechat(char *person)
{
	char ip[15];
	printf("Enter the IP of the person to chat with:");
	if(wait>1)
		{
			while (wait>1)
			{}
		}
	scanf("%s",ip);
	connectcontact (ip);

}


int exists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

void menu()
{
	int option;
	blue();
	printf("\n\n\n1.Profile settings \n2.Chat with a friend \n3.add a contact \n4.View people who are online \nany other key to exit\n");
	reset();
	if(wait>1)
		{
			while (wait>1)
			{}
		}
	scanf("%d",&option);
	if(option == 1)
		profile();
	else if(option == 2)
		pcontacts();
	else if(option == 3)
		addcontact();
	else if(option == 4)
	{	system("clear");
	green();
	printf("-----------------------------------------------------------------------------------------------------------------------------------------------------------");
	printf("\nScaning all the ip's in the lan...\n");	
	system("./temp");
	printf("Scan successfully completed");
	printf("-----------------------------------------------------------------------------------------------------------------------------------------------------------");
	reset();
	menu();
		}
	else 
		exit(0);
	return;
}

void welcome()
{
	yellow();
	printf("\n\n \t\t\t\t\t\t WELCOME TO THE COMMUNITY\t\t\t\t \n\n");
	reset();
}

void profile()
{
	int option;
	system("clear");
	
	printf("\n1. Change Username \n2. Change pin \n any other key to go back to menu");
	if(wait>1)
		{
			while (wait>1)
			{}
		}
	scanf("%d",&option);
	
	if(option == 1)
		CUN();
	else if(option == 2)
		Cpin();
	else 
		menu();
}

void CUN()
{	pflag =1;
	FILE *f;
	remove("username.txt");
	printf("Please enter the new user name:");
	scanf("%s",name);		
  	if (strlen(name) > 32 || strlen(name) < 2){
		printf("Name must be less than 30 and more than 2 characters.\n");
		return;
		}
  	f = fopen("username.txt","w");
  	fprintf(f,"%s",name);
  	printf("\nUsername successfully changed to %s",name);
  	pflag=0;
  	menu();
}

void Cpin()
{	pflag = 1;
	int key,i=0;
	char pwd[8],pin[8],pswd[8],a;
	printf("Enter the 4 digit pin to change your pin: ");
	while(1)
        {
                a = getch();
                if(a=='\n')
		{
			pwd[i] = '\0';
			break;
		}
                pwd[i++] = a;
        }
	
	FILE *x;
	x=fopen("pin.txt","r");
	fseek(x,-4,SEEK_END); 	
	fgets(pin,5,x);
	fclose(x);
	
	if((atoi(pwd)) == atoi(pin))
		{
			
				x=fopen("pin.txt","a");
				printf("Enter the new 4 digit pin: ");
				scanf("%s",pswd);
				fprintf(x,pswd);
				fclose(x);
			printf("\nPin successfully changed.");
		}
	else
		{
		printf("Wrong pin...");
		profile();
		}
		pflag = 0;
}

void username(){
	
	FILE *f;
	if(f = fopen("username.txt","r"))
	{
		fgets(name,32,f);
		
	}
	
	else
	{
		f = fopen("username.txt","w");
		printf("Please enter your name: ");
  		fgets(name, 32, stdin);
  		
  		if (strlen(name) > 32 || strlen(name) < 2){
			printf("Name must be less than 30 and more than 2 characters.\n");
			return;
			}
  		
  		str_trim_lf(name, strlen(name));
  		fprintf(f,"%s",name);
  		
	}
	
}

int main()
{	int customerlist,customeradd,adm,key;
	welcome();
	username();
	pthread_t application;
	pthread_create( &application , NULL ,  login , NULL);
	pthread_join( application , NULL);
	
return 0;
}

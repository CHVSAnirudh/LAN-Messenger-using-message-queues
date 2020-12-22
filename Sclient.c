#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#define FAIL    -1
#define LENGTH 2048
 char name[] = "Anirudh";
int server;
SSL *ssl;
int OpenConnection(const char *hostname, int port)
{
    int sd;
    struct hostent *host;
    struct sockaddr_in addr;
    if ( (host = gethostbyname(hostname)) == NULL )
    {
        perror(hostname);
        abort();
    }
    sd = socket(PF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = *(long*)(host->h_addr);
    if ( connect(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
        close(sd);
        perror(hostname);
        abort();
    }
    return sd;
}
SSL_CTX* InitCTX(void)
{
    SSL_METHOD *method;
    SSL_CTX *ctx;
    OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
    SSL_load_error_strings();   /* Bring in and register error messages */
    method = TLSv1_2_client_method();  /* Create new client-method instance */
    ctx = SSL_CTX_new(method);   /* Create new context */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}
void ShowCerts(SSL* ssl)
{
    X509 *cert;
    char *line;
    cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
    if ( cert != NULL )
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);       /* free the malloc'ed string */
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);       /* free the malloc'ed string */
        X509_free(cert);     /* free the malloc'ed certificate copy */
    }
    else
        printf("Info: No client certificates configured.\n");
}

void recv_msg_handler() {
	char message[LENGTH] = {};
	//struct Queue* queuerec = (struct Queue*)malloc(sizeof(struct Queue));
	//queuerec->front = queuerec->rear = NULL;
	const char s[2] = ":";
	char *t1,*t2;
	
  while (1) {
		int receive = SSL_read(ssl, message, LENGTH);
    if (receive > 0) {
     // printf("%s", message);
      print_recv_messages(message);
      //msgpointer();
      //enQueue(queuerec,message);
      t1 = strtok(message, s);
      t2 = strtok(NULL, s);
      //if(t2!=NULL)
      //{
      //deQueue(queuerec,t1);
      //}
    } else if (receive == 0) {
			break;
    } else {
			// -1
		}
		memset(message, 0, sizeof(message));
  }
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

void send_msg_handler() {
  	char message[LENGTH] = {};
	char buffer[LENGTH + 32] = {},a;
	int i=0;	
  	
  while(1) {
    //msgpointer();
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
    //printf("%s",message);
    //str_trim_lf(message, LENGTH);
  //  msgcount++;
    
    if (strcmp(message, "exit") == 0) {
			break;
    } else {
      sprintf(buffer, "%s: %s\n", name, message);
      SSL_write(ssl, buffer, strlen(buffer));
    }

		bzero(message, LENGTH);
    bzero(buffer, LENGTH + 32);
  }}


int main(int count, char *strings[])
{
    SSL_CTX *ctx;
   
    
    char buf[1024];
    char acClientRequest[1024] = {0};
    int bytes;
    char *hostname, *portnum;
    if ( count != 3 )
    {
        printf("usage: %s <hostname> <portnum>\n", strings[0]);
        exit(0);
    }
    SSL_library_init();
   // printf("\nEnter the IP of the host to connect:");
    //scanf("%s",hostname);
    hostname=strings[1];
    //portnum=strings[1];
    portnum=strings[2];
    ctx = InitCTX();
    server = OpenConnection(hostname, atoi(portnum));
    ssl = SSL_new(ctx);      /* create new SSL connection state */
    SSL_set_fd(ssl, server);    /* attach the socket descriptor */
    if ( SSL_connect(ssl) == FAIL )   /* perform the connection */
        ERR_print_errors_fp(stderr);
    else
    {
        printf("\n\nConnected with %s encryption\n", SSL_get_cipher(ssl));
        ShowCerts(ssl);        /* get any certs */
        
        
       // SSL_write(ssl,acClientRequest, strlen(acClientRequest));   /* encrypt & send message */
        //bytes = SSL_read(ssl, buf, sizeof(buf)); /* get reply & decrypt */
        //buf[bytes] = 0;
        //printf("Received: \"%s\"\n", buf);
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
		if(0){
			printf("\nBye\n");
			//close(sockfd);
			//wait --;
			//menu();
			break;
    }}
        SSL_free(ssl);        
    }
    close(server);         
    SSL_CTX_free(ctx); 
    return 0;
}

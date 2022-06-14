#define _WINSOCK_DEPRECATED_NO_WARNINGS

/*
    Create a TCP socket
*/
 
#include <stdio.h>
#include <winsock2.h>
#include <pthread.h>
#pragma comment(lib,"ws2_32.lib") 

WSADATA wsa;
SOCKET s;
struct sockaddr_in server;
char server_reply[2000];
int recv_size;
pthread_t tid[2];
pthread_mutex_t mutex;


void *send_message(void*)		//Thread to send message to server
{
	char message[2000];
	while(1)
	{
		fgets(message,2000,stdin);
		message[strlen(message)-1]='\0';
		pthread_mutex_lock(&mutex);			
		if( send(s , message , strlen(message) , 0) < 0)
	    {
	        puts("Send failed\nConnection to Server lost!");
	        exit(1);
	    }
		if(strcmp(message,"exit")==0)
	    {
	    	closesocket(s);
    		WSACleanup();
    		pthread_mutex_destroy(&mutex);
    		printf("Connection closed. \n");
    		exit(0);
	    }
	    pthread_mutex_unlock(&mutex);
	}
}


void receive_server()		//function to receive message from server
{
	if((recv_size = recv(s , server_reply , 2000 , 0)) == SOCKET_ERROR ||recv_size==0)
	    {
	    	pthread_mutex_lock(&mutex);
	        puts("Connection to Server lost!");
	        pthread_mutex_unlock(&mutex);
			exit(1);
	    }
	 	
		 //Add a NULL terminating character to make it a proper string before printing
	    server_reply[recv_size] = '\0';
	    if(strcmp(server_reply,": exit")==0)
	    {
	    	closesocket(s);
    		WSACleanup();
    		pthread_mutex_destroy(&mutex);
    		printf(" closed the connection. \n");
    		exit(0);
	    }
	//    puts(server_reply);
	    printf("%s",server_reply);
}


void *receive_message(void*)		//Thread to receive message from server
{
	  
    while(1)
    {
    	
		receive_server();  //name of sender
		receive_server();	//the rest of the message
		printf("\n\n");
	//	receive_server();
		
		
	}
}


void initialise_winsock()
{
//	printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        printf("Failed. Error Code : %d",WSAGetLastError());
        exit(1);
    }
     
    printf("Initialised.\n");
} 

void create_socket()
{
	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d" , WSAGetLastError());
    }
 
//    printf("Socket created.\n");
}

int connect_to_server()		//function to connect to a given server
{
	char server_ip[100];
	int port=8888;
	printf("Enter IP of Server: ");
	fgets(server_ip,100,stdin);
	server_ip[strlen(server_ip)-1]='\0';
//	printf("Enter port no: ");
//	scanf("%d",&port);
	server.sin_addr.s_addr = inet_addr(server_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons( port );
 
    //Connect to remote server
    if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("connect error");
        exit(1);
    }
     
 //   puts("Connected");
  //  puts("To send a message, type it and press Enter.\n");
    receive_server();
}


int main(int argc , char *argv[])
{
    printf("Welcome! \n\n");
    initialise_winsock();
    
	create_socket();
     
    connect_to_server();
	pthread_mutex_init(&mutex,NULL);
    pthread_create(&tid[0], NULL, receive_message,NULL);
	pthread_create(&tid[1], NULL, send_message, NULL);
	for(int i=0;i<2;i++)
	{
		pthread_join(tid[i], NULL);
	}
	
    return 0;
}



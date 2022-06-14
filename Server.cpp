#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
/*
    Live Server on port 8888
*/
#include<io.h>
#include<stdio.h>
#include<string.h>
#include<winsock2.h>
#include<pthread.h>

 
#pragma comment(lib,"ws2_32.lib") //Winsock Library
#define MAX_CLIENT 20

WSADATA wsa;
SOCKET s , new_socket[MAX_CLIENT];
struct sockaddr_in server, client[MAX_CLIENT];
int c;
char welcome_message[200]="Welcome to the Chat Room! \nTo send a message, type it and press Enter. \nPlease enter your name: ";
char message[2000];
pthread_t tid[MAX_CLIENT],pid;
pthread_mutex_t mutex;
char ip[MAX_CLIENT][100], client_name[MAX_CLIENT][100];

void end_thread(int client_id)		//function to end a thread if called from within itself
{
	pthread_mutex_lock(&mutex);
	//printf("Connection to Client IP: %s  Client Name: %s   lost!",ip[client_id],client_name[client_id]);
	ip[client_id][0]='\0';
	closesocket(new_socket[client_id]);
	pthread_mutex_unlock(&mutex);
	pthread_exit(NULL);
	
}
void receive_message(int client_id)		//function to receive a message from the given client_id
{
	//SOCKET s=*(int*)socket_pointer
	//char reply[2000];
    int recv_size;
    
		if((recv_size = recv(new_socket[client_id], message , 2000, 0)) == SOCKET_ERROR || recv_size==0)
	    {
	   // 	printf("its in %d",recv_size);
	    	pthread_mutex_lock(&mutex);
	        printf("Connection to Client IP: %s  Client Name: %s  lost! \n",ip[client_id],client_name[client_id]);
	        pthread_mutex_unlock(&mutex);
			end_thread(client_id);
	    }
	    // getchar();
	    //puts("Reply received\n");
	 	//printf("%d",recv_size);
	    //Add a NULL terminating character to make it a proper string before printing
	    message[recv_size] = '\0';
	    if(strcmp(message,"exit")==0 )
	    {
	    	printf("Connection closed by the client with IP %s \n",ip[client_id]);
	    	//getchar();
			end_thread(client_id);
	    }
	    printf("%s: ",client_name[client_id]);
	    puts(message);
}


void send_message(int client_id)	//function to send a message to all except the given client id
{
	
		for(int i=0;i<MAX_CLIENT;i++)
		{
			pthread_mutex_lock(&mutex);
			if(i!=client_id && strlen(ip[i])!=0 && client_id!=-1)
			{
			
				if( send(new_socket[i] , client_name[client_id] , strlen(client_name[client_id]) , 0) >=0)
			    {
			    	send(new_socket[i] , ": " , strlen(": ") , 0);
			    	send(new_socket[i] , message , strlen(message) , 0);
			    }
			    else
				{
			    	puts("Send failed");
			    //	end_thread(i);
				}
			}
			else if (strlen(ip[i])!=0 && client_id==-1)
			{
				if( send(new_socket[i] , "Server" , strlen("Server") , 0) >=0)
			    {	
			    	send(new_socket[i] , ": " , strlen(": ") , 0);
			    	send(new_socket[i] , message , strlen(message) , 0);
			    }
			    else
				{
			    	puts("Send failed");
			    //	end_thread(i);
				}
			}
		//	if(strcmp(message,"exit")==0)
	//	  	{
	//		closesocket(new_socket[i]);
    		//WSACleanup();
    		//pthread_mutex_destroy(&mutex);
    //		printf("Connection to Client IP: %s  Client Name: %s  terminated! \n",ip[i],client_name[i]);
    		//pthread_cancel(tid[i]);
    		//exit(0);
	  //  	}
			
			pthread_mutex_unlock(&mutex);
		}
	
	
   // puts("Data Send\n");
}



void welcome_client(int client_id)	//function to welcome a client and ask its name
{
	char cl[100];
	_itoa(client_id+1, cl, 10);
	strcpy(client_name[client_id],"Client ");
	client_name[client_id][strlen(client_name[client_id])]=*cl;
	client_name[client_id][strlen(client_name[client_id])]='\0';
	
	
	send(new_socket[client_id] , welcome_message , strlen(welcome_message) , 0);
    receive_message(client_id);
    
    
    strcpy(client_name[client_id], message);
}

    
void *newclient(void *client_no)	//thread to serve whenever a newclient joins the server
{
	//SOCKET new_socket= *(SOCKET*)s;
	//SOCKET s=(SOCKET)new_socket;
	int client_id=*(int*) client_no;
	puts("Connection accepted");
    
    strcpy(ip[client_id],inet_ntoa(client[client_id].sin_addr));
    printf("Connected to %s \n",ip[client_id]);
   // Reply to the client
    //char *message;
    welcome_client(client_id);
//    printf("client_id is %d",client_id);
 	while(1)
 	{
     	receive_message(client_id);
     	send_message(client_id);
    }
}


void initialise_winsock()
{
	printf("\nInitialising Winsock...");
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
 
    printf("Socket created.\n");
}

void bind()
{
	//Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
     
    //Bind
    if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
    {
        printf("Bind failed with error code : %d" , WSAGetLastError());
        exit(EXIT_FAILURE);
    }
     
    puts("Bind done");
} 

void *send_message_to_all(void *)	//thread created to send a message to all clients from server 
{
	//SOCKET new_socket= *(SOCKET*)s;
	//SOCKET s=(SOCKET)new_socket;
	
 	while(1)
 	{
 		fgets(message,2000,stdin);
		message[strlen(message)-1]='\0';
     	send_message(-1);
	}
}


void accept_connections()		//function to accept new connections
{
	//Listen to incoming connections
    listen(s , 3);
    int client_id=-1;
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
     
    c = sizeof(struct sockaddr_in);
    while((new_socket[client_id+1] = accept(s , (struct sockaddr *)&client[client_id+1], &c)) != INVALID_SOCKET)    
    {
    	if (client_id+1<MAX_CLIENT)
    		pthread_create(&tid[++client_id],NULL, newclient, &client_id);
		else
		{	
			send(new_socket[client_id+1] , "Server capacity full! \n" , strlen("Server capacity full! \n") , 0);
    		closesocket(new_socket[client_id+1]);
    	}
		//no_of_clients++;
    }
    if(new_socket[client_id+1]==INVALID_SOCKET)
    	printf("accept failed with error code : %d" , WSAGetLastError());
    
//    if( client_id+1==MAX_CLIENT)
  //   	printf("Fuckkkk!");
	for(int i=0;i<=client_id;i++)
	{
		pthread_join(tid[i], NULL);
	}
	closesocket(s);
    WSACleanup();
}


int main(int argc , char *argv[])
{
	initialise_winsock();
    
	create_socket();    
 
	bind();   
	
    pthread_mutex_init(&mutex,NULL);
	pthread_create(&pid,NULL, send_message_to_all, NULL); 
    accept_connections();
    pthread_mutex_destroy(&mutex);
	return 0;
}

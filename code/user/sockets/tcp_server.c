/*
    C socket server example, handles multiple clients using threads
    Compile
    gcc server.c -lpthread -o server
*/
 
#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread

#define MY_PORT 8888 

//the thread function
void *connection_handler(void *);
 

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
    int my_port = MY_PORT;
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( my_port );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    pthread_t thread_id;
	
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
      {
      puts("Connection accepted");
         
      if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
        puts("Handler assigned");
    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
     
    return 0;
}
 
/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{

    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char message[132] , client_message[2000];
    pthread_t my_id = pthread_self();

    pthread_detach(my_id);

    //Send some messages to the client
    snprintf(message, sizeof(message)
	     ,"Greetings from %ld! I am your connection handler\n"
	     , (long)my_id);

    write(sock , message , strlen(message));
     
    snprintf(message, sizeof(message)
	     ,"[%ld] Now type something and I shall repeat what you type \n"
	     , (long)my_id);
    write(sock, message, strlen(message));
     
    //Receive a message from client
    while( (read_size = recv(sock, client_message, sizeof(client_message)-1 , 0)) > 0 )
    {
      //end of string marker
      client_message[read_size] = '\0';
		
      //Send the message back to client
      write(sock, client_message, strlen(client_message));
      
      //clear the message buffer
      memset(client_message, 0, 2000);
    }
    
    if(read_size == 0)
      {
        puts("Client disconnected");
        fflush(stdout);
      }
    else if(read_size == -1)
      {
        perror("recv failed");
      }
         
    return 0;
} 

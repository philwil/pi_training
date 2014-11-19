/*
    C socket server example, handles multiple clients using threads
    Compile
    if -d option is given, reads data from the driver 
    and sends that to the connection
    if -s option is given reads data from client and sends that to the driver
    gcc tcp_server.c -lpthread -o tcp_server
*/
 
#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MY_PORT 8888 

//the thread function
void *connection_handler(void *);
 
int s_flag = 0;
int d_flag = 0;
int fd=-1;

int main(int argc , char *argv[])
{
  int flags, opt;
  int port;
  char *device = NULL;
  int my_port = MY_PORT;

  while ((opt = getopt(argc, argv, "sdp:D:")) != -1) {
        switch (opt) {
        case 's':
            s_flag = 1;
            break;
        case 'd':
            d_flag = 1;
            break;
        case 'p':
            my_port = atoi(optarg);
            //tfnd = 1;
            break;

        case 'D':
            device = optarg;
            //tfnd = 1;
            break;

        default: /* '?' */
            fprintf(stderr, "Usage: %s [-p port] -s -d -D device_name\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    fd=-1;

    int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
    
    char *client_ip;
    int client_port;

    if(device != NULL)
      {
	flags = O_RDWR;
	fd = open(device, flags);
      }
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
	
    while((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
      {
	client_ip = inet_ntoa(client.sin_addr);
	client_port = ntohs(client.sin_port);
	
	printf("Connection accepted from [%s] port (%d)"
	       , client_ip, (int)client_port);
	
	if( pthread_create(&thread_id, NULL
			   ,  connection_handler, (void*) &client_sock) < 0)
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
     
    if(fd>=0) close(fd);

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
    int rc;
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
    while((fd >= 0) && (s_flag==1))
      {
	read_size = read(fd, client_message, sizeof(client_message)-1);
	if (read_size > 0)
	  {
	    rc = write(sock, client_message, read_size);
	  }
	else
	  {
	    fd = -1;
	    close(sock);
	    sock = -1;
	  }
      }     

    while((fd >= 0) && (d_flag==1))
      {
	read_size = recv(sock, client_message, sizeof(client_message), 0);

	if (read_size > 0)
	  {
	    rc = write(fd, client_message, read_size);
	  }
	else
	  {
	    close(fd);
	    fd = -1;
	    close(sock);
	    sock = -1;
	  }
      }
    read_size = 0;
    if(sock > 0)
      {
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

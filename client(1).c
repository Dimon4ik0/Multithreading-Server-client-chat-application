// Client side C program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<stdlib.h>
#include <unistd.h>
#include<pthread.h>
#define PORT 8080
#define BUFFER_SIZE 1024

int server_socket;
char buffer[BUFFER_SIZE];

void* receive_messages(void* arg)
{
  while(1)
  { 
    memset(buffer,0,BUFFER_SIZE);
    int bytes_received = read(server_socket,buffer,BUFFER_SIZE);
    if(bytes_received <= 0)
    {
      printf("Disconnected from server.\n");
      close(server_socket);
      pthread_exit(NULL);
    }
    printf("\nServer: %s\n",buffer);
  }
  return NULL;
}



int main(int argc, char const* argv[])
{
   

    

     server_socket = socket(AF_INET,SOCK_STREAM,0);
    if(server_socket == -1)
    {
      printf("cannot create socket\n");
      return 0;
    }
    printf("socket created\n");
    
    struct sockaddr_in client;
    memset(&client,0,sizeof(client));
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr("127.0.0.1");
    client.sin_port = htons(PORT);
    
    if(connect(server_socket,(struct sockaddr*)&client, sizeof(client)) != 0)
    {
      printf("cannot connect to server!\n");
      close(server_socket);
      return 0;
    }
    printf("connected to server\n");
    
    printf("Welcome to Prime and Not Prime numbers. Please, enter your number\n");
    
    pthread_t receive_thread;
    if(pthread_create(&receive_thread,NULL, receive_messages,NULL) != 0) {
    printf("Failed to create thread\n");
    close(server_socket);
    return 0;
    }
    
    
    
    while(1)
    {
      printf("Client: ");
      fflush(stdout);
      memset(buffer,0,BUFFER_SIZE);
      fgets(buffer,BUFFER_SIZE,stdin);
      buffer[strcspn(buffer,"\n")] = '\0';
      
      if(strcmp(buffer,"exit") == 0)
      {
        printf("Exiting the program\n");
        break;
      }
      
      send(server_socket,buffer,strlen(buffer) + 1, 0);
      
    
      
    }
    
   pthread_cancel(receive_thread);
   pthread_join(receive_thread,NULL);
   close(server_socket);
    return 0;
  }
    
  
    
    
 
        
        

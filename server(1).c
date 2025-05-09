#include<pthread.h>
#include<semaphore.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<string.h>
#include<math.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>
#include<errno.h>
#include<time.h>
#include<signal.h>
#define PORT 8080
#define BUFFER_SIZE 1024
//pipes
#define READ_END 0
#define WRITE_END 1
//Timer
#define TIMERSEC 20
timer_t casovac;

int pipe1[2],pipe2[2];
int number_of_clients = 0;

pthread_mutex_t pipe_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t client_count_lock = PTHREAD_MUTEX_INITIALIZER;

/*timer_t vytvorCasovac(int signal) {
  struct sigevent kam;
  kam.sigev_notify = SIGEV_SIGNAL;
  kam.sigev_signo = signal;
  
  timer_t casovac;
  timer_create(CLOCK_REALTIME, &kam,&casovac);
  return(casovac);
}


void spustiCasovac(timer_t casovac, int sekundy) { 
  struct itimerspec casik;
  casik.it_value.tv_sec = sekundy;
  casik.it_value.tv_nsec = 0;
  casik.it_interbal.tv_sec = 0;
  casik.it_interval.tv_nsec = 0;
  timer_settime(casovac,CLOCK_REALTIME,&casik,NULL);
}*/




int is_prime(int num) {

 if(num <= 1) {
    return 0;
  }
  for(int i = 2; i*i <= num; i++) {
    if(num % i == 0) {
      return 0;
    }
  }
    return 1;
}



void* handle_client(void* arg) {

  char *endptr;
  int client_socket = *(int *)arg;
  free(arg);
  char buffer[BUFFER_SIZE];
  ssize_t bytes_read;
  
  write(client_socket,"Hello! Welcome to the server.\n",30);
  
  while(1) {
  
    memset(buffer,0,BUFFER_SIZE);
    bytes_read = read(client_socket,buffer,BUFFER_SIZE - 1);
    if(bytes_read <= 0) {
    
      close(client_socket);
      pthread_mutex_lock(&client_count_lock);
      number_of_clients--;
      pthread_mutex_unlock(&client_count_lock);
      printf("Client has disconnected. Active clients: %d\n", number_of_clients);
      pthread_exit(NULL);
    }



    buffer[bytes_read] = '\0';
    printf("Client: %s\n",buffer);
    
    if(strcmp(buffer,"exit") == 0) {
    
      close(client_socket);
      pthread_mutex_lock(&client_count_lock);
      number_of_clients--;
      pthread_mutex_unlock(&client_count_lock);
      printf("Client requested to exit. Active clients: %d\n",number_of_clients);
      pthread_exit(NULL);
    }
    
    int num = strtol(buffer,&endptr,10);
    
   pthread_mutex_lock(&pipe_mutex);
    
    write(pipe1[WRITE_END], &num, sizeof(num));
    
    int result;
    
    read(pipe2[READ_END], &result, sizeof(result));
    
  pthread_mutex_unlock(&pipe_mutex);
    
    if(result == 1) {
      write(client_socket,"Prime",strlen("Prime"));
    }
    if(result == 0) {
      write(client_socket,"Not Prime",strlen("Not Prime"));
    }
    
    printf("The client passed the value %d. Result %s\n",num,result == 1? "Prime" : "Not Prime");
    }
    
    
    return NULL;
  }
  


    
    
    
  
  

int main(int argc, char const* argv[])
{
    printf("Welcome to the multi-threaded Prime Numbers Detector\n");
    int server_socket;
    struct sockaddr_in server;
    pthread_t thread_id;
    
    
    server_socket = socket(AF_INET,SOCK_STREAM,0);
    //create socket
    if(server_socket == -1){
      printf("cannot create socket!\n");
      return 0;
    }
    printf("socket is created\n");
    //here socket is created
    
    
    memset(&server,0,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    
    if(bind(server_socket,(struct sockaddr*)&server, sizeof(server)) != 0){
      printf("cannot bind socket\n");
      close(server_socket);
      return 0;
    }
    printf("bind was successful\n");
    
    if(listen(server_socket,20) != 0){
      printf("cannot listen on socket\n");
      close(server_socket);
      return 0;
    }
    printf("listen was successful\n");
    
    if(pipe(pipe1) == -1 || pipe(pipe2) == -1) {
    
      perror("Pipe creation error");
      exit(EXIT_FAILURE);
    }
    
    
    
    printf("server is running waiting for the clients to connect\n");
    
    pid_t child_pid = fork();
    
    if(child_pid == -1)
    {
      perror("Error when creating a child process");
      return 0;
    }
    //Child process
    if(child_pid == 0) {
    
      close(pipe1[WRITE_END]);
      close(pipe2[READ_END]);
      
      int number,result;
      
      while(1) {
      if(read(pipe1[READ_END],&number,sizeof(number)) <= 0) {
        break;
      }
      result = is_prime(number);
      write(pipe2[WRITE_END],&result,sizeof(result));
      
    }
    close(pipe1[READ_END]);
    close(pipe2[WRITE_END]);
    exit(0);
    }
    //Parent process
    else {
      close(pipe1[READ_END]);
      close(pipe2[WRITE_END]);
          
    while(1)
    {
    
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    memset(&client,0,sizeof(client));
    
    int* client_socket = malloc(sizeof(int));
    if(!client_socket) {
      perror("Memory allocation failed");
      continue;
    }
    
    
    
    
    *client_socket = accept(server_socket,(struct sockaddr*)&client,&len);
    if(*client_socket == -1){
      printf("cannot accept client\n");
      free(client_socket);
      continue;
    }
    
    
    number_of_clients++;

    printf("New client connected. Active clients: %d\n",number_of_clients);
    
    
    if(pthread_create(&thread_id,NULL,handle_client,client_socket) == 0) {
    
      pthread_detach(thread_id);
    }
    else {
    
      perror("Thread creation failed");
      free(client_socket);
      close(*client_socket);
      
    
    }
   }
 }
  
  close(server_socket);
  return EXIT_SUCCESS;
}


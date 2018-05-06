/*
    Program for a chat server
    It uses sockets, threads (paralelism), signals and __moreinfohere__

    Ivan Aram Gonzalez Su - A01022584
    Jose Manuel Beauregard Mendez - A01021716
    Sebastian Galguera Ortega - A01016708
*/

/*
    Program for a simple bank server
    It uses sockets and threads
    The server will process simple transactions on a limited number of accounts

    Ivan Aram Gonzalez Su
    A01022584
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// Signals library
#include <errno.h>
#include <signal.h>
// Sockets libraries
#include <netdb.h>
#include <sys/poll.h>
// Posix threads library
#include <pthread.h>

// Custom libraries
#include "sockets.h"
#include "fatal_error.h"
#include "codes.h"

#define BUFFER_SIZE 1024
#define MAX_QUEUE 5
#define NUM_CHAT_TABLES 3
#define MAX_MESSAGES_PER_TABLE 10

typedef enum {false, true} bool;

typedef struct {
  char *name;
  int topic;
} chat_t;

typedef struct {
  char *topic;
  int numOfChats;
  int numOfMessages;
  char *key;
  char *messages[MAX_MESSAGES_PER_TABLE];
} chat_table_t;

typedef struct {
  chat_t *chat;
  int client_fd;
  chat_table_t *tables;
} thread_data_t;

///// FUNCTION DECLARATIONS
void usage(char *);
void setupHandlers();
void interruptionHandler(int);
void waitForConnections(int, chat_table_t *);
void *attentionThread(void *);
void initChatTables(chat_table_t *);

///// MAIN FUNCTION
int main(int argc, char *argv[]){
    int server_fd;
    chat_table_t tables[NUM_CHAT_TABLES];

    // Check the correct arguments
    if (argc != 2){
        usage(argv[0]);
    }
    printf("\n=== CHAT SERVER ===\n");
    // Configure the handler to catch SIGINT
    setupHandlers();

    // Show the IPs assigned to this computer
    printLocalIPs();

    initChatTables(tables);

    // Start the server
    server_fd = initServer(argv[1], MAX_QUEUE);
	
	// Listen for connections from the clients
    waitForConnections(server_fd, tables);
    
    // Close the socket
    close(server_fd);

    return 0;
}

///// FUNCTION DEFINITIONS
/*
    Explanation to the user of the parameters required to run the program
*/
void usage(char *program){
    printf("Usage:\n");
    printf("\t%s {port_number}\n", program);
    exit(EXIT_FAILURE);
}

/*
    Modify the signal handlers for specific events
*/
void setupHandlers(){
    struct sigaction new_action;

    // Handle Ctrl-c
    new_action.sa_handler = interruptionHandler;
    new_action.sa_flags = SA_RESETHAND;

    // Apply the handler
    sigaction(SIGINT, &new_action, (struct sigaction *)NULL);
}

/*
    Function to handle Ctrl-C (interruption)
*/
void interruptionHandler(int sig){

}

/*
    Main loop to wait for incomming connections
*/
void waitForConnections(int server_fd, chat_table_t *_tables){
  struct sockaddr_in client_address;
  socklen_t client_address_size;
  char client_presentation[INET_ADDRSTRLEN];
  int client_fd;
  int thread_status;
  int poll_response;
	int timeout = 500; // Time in milliseconds (0.5 seconds)

    // Get the size of the structure to store client information
    client_address_size = sizeof client_address;

    while (1){
		//// POLL
        // Create a structure array to hold the file descriptors to poll
        struct pollfd test_fds[1];
        // Fill in the structure
        test_fds[0].fd = server_fd;
        test_fds[0].events = POLLIN;    // Check for incomming data
        // Check if there is any incomming communication
        poll_response = poll(test_fds, 1, timeout);

		    // Error when polling
        if (poll_response == -1){
            // Test if the error was caused by an interruption
            if (errno == EINTR){
              // INTERRUPTION
              //printf("Poll did not finish. The program was interrupted");
            }
            else{
                fatalError("ERROR: poll");
            }
        }
		    // There is something ready at the socket
        else{
            // Check the type of event detected
            if (test_fds[0].revents & POLLIN){
    			// ACCEPT
    			// Wait for a client connection
    			client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_address_size);
    			if (client_fd == -1){
    				fatalError("ERROR: accept");
    			}
    
    			// Get the data from the client
    			inet_ntop(client_address.sin_family, &client_address.sin_addr, client_presentation, sizeof client_presentation);
    			printf("Received incomming connection from %s on port %d\n", client_presentation, client_address.sin_port);
                
          // Prepare the structure to send to the thread
					thread_data_t thread_data;
					thread_data.tables = _tables;
					thread_data.client_fd = client_fd;

					// Create thread
          pthread_t client_thread;
          thread_status = pthread_create(&client_thread, NULL, &attentionThread, &thread_data);
          if(thread_status != 0){
            perror("ERROR: pthread_create");
            exit(EXIT_FAILURE);
          }
        }
      }
    }
}

/*
    Hear the request from the client and send an answer
*/
void *attentionThread(void *arg){
	char buffer[BUFFER_SIZE];
  thread_data_t *data = arg;
  chat_t chat;
  data->chat = &chat;
  int operation;
  response_t response;
  int number = -1;
  
  // Receive clients name
  if ( !recvString(data->client_fd, buffer, BUFFER_SIZE) ) {
    printf("Error recv\n");
    pthread_exit(NULL);
  }
  sscanf(buffer, "%d %s", &operation, chat.name);
  // Main while 
  while(operation != EXIT){
  	if(number >= 0){
  		if ( !recvString(data->client_fd, buffer, BUFFER_SIZE) ) {
    	    printf("Error recv\n");
    	    pthread_exit(NULL);
    	}
		sscanf(buffer, "%d %d", &operation, &number);
  	}
  	//RECIEVE STRING
  	switch(operation){
  		case NAME:
  			response = OK;
  			sprintf(buffer, "%d %d", response, NUM_CHAT_TABLES);
  			sendString(data->client_fd, buffer);
  			sprintf(buffer, "%s %s %s", data->tables[0].topic, data->tables[1].topic, data->tables[2].topic);
  			sendString(data->client_fd, buffer);
  			break;
  		case TOPIC:
  			response = KEY;
  			chat.topic = number;
  			
  			sprintf(buffer, "%d 0", response);
  			sendString(data->client_fd, buffer);

  			for (int i = 0; i < NUM_CHAT_TABLES; i++) {
  			    if(chat.topic == i){
  			        sprintf(buffer, "%s", data->tables[i].key);
  			        sendString(data->client_fd, buffer);
  			        break;
  			    }
  			}
  			break;
  		case SEND:
  		    //char message[BUFFER_SIZE]; 
  		    if ( !recvString(data->client_fd, buffer, BUFFER_SIZE) ) {
        	    printf("Error recv\n");
        	    pthread_exit(NULL);
        	}
        	
        	//sscanf(buffer, "%s", message); // GET MESSAGE, MAYBE NOT NECESSARY BECAUSE buffer HAS THE MSG
        	
        	for (int i = 0; i < NUM_CHAT_TABLES; i++) {
  			    if(chat.topic == i){
  			        if(data->tables[i].numOfMessages < MAX_MESSAGES_PER_TABLE){
  			            data->tables[i].messages[data->tables[i].numOfMessages++] = buffer;
  			            //data->tables[i].messages[data->tables[i].numOfMessages++] = message;
  			        } else{
  			            for (int j = 0; j < MAX_MESSAGES_PER_TABLE; j++) {
  			                if(j != MAX_MESSAGES_PER_TABLE - 1){
  			                    data->tables[i].messages[j] = buffer;
  			                    //data->tables[i].messages[j] = message;
  			                } else {
  			                    data->tables[i].messages[j] = data->tables[i].messages[j + 1];
  			                }
  			            }
  			        }
  			        break;
  			    }
  			}
        	
  			break;
  		case SHOW:
  		    for (int i = 0; i < NUM_CHAT_TABLES; i++) {
  		        if(chat.topic == i){
  		            response = MESSAGES;
  		            sprintf(buffer, "%d %d", response, data->tables[i].numOfMessages);
  		            sendString(data->client_fd, buffer);
  		            for (int j = 0; j < data->tables[i].numOfMessages; j++) {
  		                //sprintf(buffer, "%s", data->tables[i].messages[j]);
  		                sendString(data->client_fd, data->tables[i].messages[j]);
  		            }
  		            break;
  		        }
  		    }
  			break;
  		case EXIT:
  		    response = BYE;
  		    sprintf(buffer, "%d 0", response);
  			sendString(data->client_fd, buffer);
  			break;
  	}
  	number = 0;
  }
  pthread_exit(NULL);
}

/*
    Initialize chat tables
*/
void initChatTables(chat_table_t *chat_tables){
  for (size_t i = 0; i < NUM_CHAT_TABLES; i++) {
    chat_tables[i].numOfChats = 0;
    chat_tables[i].numOfMessages = 0;
    if(i == 0){
        chat_tables[i].topic = "Math";
        chat_tables[i].key = "encriptionkey1";
    }
    else if(i == 1){
        chat_tables[i].topic = "Nezfliz";
        chat_tables[i].key = "encriptionkey2";
    }
    else if(i == 2){
        chat_tables[i].topic = "Chess";
        chat_tables[i].key = "encriptionkey2";
    }
  }
}
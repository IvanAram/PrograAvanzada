/*
    Program for a chat server
    It uses sockets, threads, signals, a communication protocol and encription

    Ivan Aram Gonzalez Su - A01022584
    Jose Manuel Beauregard Mendez - A01021716
    Sebastian Galguera Ortega - A01016708
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

///// CONSTANTS DECLARATIONS
#define MAX_QUEUE 5
#define BUFFER_SIZE 2048
#define NUM_CHAT_TABLES 3
#define MAX_MESSAGES_PER_TABLE 10

typedef enum {false, true} bool;

typedef struct {
  char name[50];
  int topic;
} chat_t;

typedef struct {
  char *topic;
  int numOfChats;
  int numOfMessages;
  char *key;
  char messages[MAX_MESSAGES_PER_TABLE][BUFFER_SIZE];
} chat_table_t;

typedef struct {
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
  printf("\nServer interrupted\n");
  exit(EXIT_SUCCESS);
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
  // this array will save string that will send to the client
  char buffer[BUFFER_SIZE];
  // this will save the information of the connection with a client
  thread_data_t data = *(thread_data_t *)arg;
  // this will save the information of the chats
  chat_t chat;
  // this will save the current status of client-server
  int operation;
  // same as +opertaion+ but server-client
  response_t response;
  // helper variable that will save information from a request
  int number = -1;

  // Receive clients name
  if ( !recvString(data.client_fd, buffer, BUFFER_SIZE) ) {
    printf("Error recv\n");
    pthread_exit(NULL);
  }
  sscanf(buffer, "%d %s", &operation, chat.name);

  // Main while
  while(operation != EXIT){
    if(number >= 0){
      if ( !recvString(data.client_fd, buffer, BUFFER_SIZE) ) {
        printf("Error recv\n");
        pthread_exit(NULL);
    	}
	    sscanf(buffer, "%d %d", &operation, &number);
      //printf("OPERATION: %d\nNUMBER: %d\n", operation, number);
  	}
  	// RECIEVE DIFFERENT RESPONSE FROM CLIENT
  	switch(operation){
  		case NAME:
        // When client response with NAME, send amount of chat tables
  			response = OK;
  			sprintf(buffer, "%d %d", response, NUM_CHAT_TABLES);
  			sendString(data.client_fd, buffer);
        // we sleep 1 sec after each send (if theres another send next to it)
        // so the client has time to recieve each request without breaking
        sleep(1);
  			sprintf(buffer, "%s %s %s", data.tables[0].topic, data.tables[1].topic, data.tables[2].topic);
        sendString(data.client_fd, buffer);
  			break;
  		case TOPIC:
        // When user selects a topic/table, send the key for encryption/decryption
  			response = KEY;
  			chat.topic = number;

  			sprintf(buffer, "%d 0", response);
  			sendString(data.client_fd, buffer);
        sleep(1);
        sprintf(buffer, "%s", data.tables[chat.topic].key);
        sendString(data.client_fd, buffer);
  			break;
  		case SEND:
        // When user wants to send a message, append it to the currect table
		    if ( !recvString(data.client_fd, buffer, BUFFER_SIZE) ) {
    	    printf("Error recv\n");
    	    pthread_exit(NULL);
      	}

        if(data.tables[chat.topic].numOfMessages < MAX_MESSAGES_PER_TABLE){
          sscanf(buffer, "%s", data.tables[chat.topic].messages[data.tables[chat.topic].numOfMessages]);
          data.tables[chat.topic].numOfMessages += 1;
        } else{
          for (int j = 0; j < MAX_MESSAGES_PER_TABLE; j++) {
            if(j == MAX_MESSAGES_PER_TABLE - 1){
              sscanf(buffer, "%s", data.tables[chat.topic].messages[j]);
            } else {
              strcpy(data.tables[chat.topic].messages[j], data.tables[chat.topic].messages[j + 1]);
            }
          }
        }

  			break;
  		case SHOW:
        // User wants to see all the messages of an the selected chat_room
        response = MESSAGES;
        sprintf(buffer, "%d %d", response, data.tables[chat.topic].numOfMessages);
        sendString(data.client_fd, buffer);
        // We send each message with its own request
        for (int j = 0; j < data.tables[chat.topic].numOfMessages; j++) {
          sleep(1);
          sendString(data.client_fd, data.tables[chat.topic].messages[j]);
        }
  			break;
  		case EXIT:
        // Client will disconnect
        printf("Client left\n");
		    response = BYE;
		    sprintf(buffer, "%d 0", response);
  			sendString(data.client_fd, buffer);
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
        // we use this key to encrypt/decrypt messages
        chat_tables[i].key = "bitches";
    }
    else if(i == 1){
        chat_tables[i].topic = "Nezfliz";
        chat_tables[i].key = "seviches";
    }
    else if(i == 2){
        chat_tables[i].topic = "Chess";
        chat_tables[i].key = "jajajiel";
    }
  }
}

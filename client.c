/*
    Program for a chat client
    It uses sockets, threads, signals, a communication protocol and encription

    Ivan Aram Gonzalez Su - A01022584
    Jose Manuel Beauregard Mendez - A01021716
    Sebastian Galguera Ortega - A01016708
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// Sockets libraries
#include <netdb.h>
#include <arpa/inet.h>
// Signals library
#include <errno.h>
#include <signal.h>
// Custom libraries
#include "sockets.h"
#include "fatal_error.h"
#include "codes.h"
#include "blowfish.h"

///// CONSTANTS DECLARATIONS
#define BUFFER_SIZE 2048
#define MAX_NAME_SIZE 50

///// FUNCTION DECLARATIONS
void usage(char *);
void setupHandlers();
void interruptionHandler(int);
void chatOperations(int);

int connection_fd;

///// MAIN FUNCTION
int main(int argc, char *argv[]){

	// Check the correct arguments
	if (argc != 3){
		usage(argv[0]);
	}

	setupHandlers();

	// Start the server connection
	connection_fd = connectSocket(argv[1], argv[2]);

	// Use the bank operations available
	printf("\n=== CHAT CLIENT PROGRAM ===\n");

	chatOperations(connection_fd);
	// Close the socket
	close(connection_fd);

	return 0;
}

///// FUNCTION DEFINITIONS
/*
    Explanation to the user of the parameters required to run the program
*/
void usage(char *program){
	printf("Usage:\n");
	printf("\t%s {server_address} {port_number}\n", program);
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
void interruptionHandler(int signal) {
	char lastBuffer[5];
	sprintf(lastBuffer, "%d 0", EXIT);
	sendString(connection_fd, lastBuffer);
	printf("\nThanks for using our chat tables!\n");
	exit(EXIT_SUCCESS);
}

/*
    Main menu with the options available to the user
*/
void chatOperations(int connection_fd) {
	// array thats saves server response
	char buffer[BUFFER_SIZE];
	// array that stores client's name
	char name[MAX_NAME_SIZE];
	// our chats are filtered by a topics that are saved in this array
	char topics[3][MAX_NAME_SIZE];
	// this will save the client's +chat_room+ selection
	int chat_room = 4;
	// this will save user's action in a +chat_room+
	char opt = 'a';
	// this will save the current status of client-server
	int operation = NAME;
	// key value to encrypt/decrypt the text (each +chat_room+ has it's own key)
	char KEY_VAL[20];
	// array with user's message
	char message[BUFFER_SIZE];
	// same as +opertaion+ but server-client
	int response = OK;
	// amount of messages in the +chat_room+
	int numOfMessages;

	// Start interaction with user's client
	printf("Enter your display name: ");
	scanf("%s", name);
	// Write message to server
	sprintf(buffer, "%d %s", operation, name);
	sendString(connection_fd, buffer);

	// Recieve response of the user creation
	if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ) {
    printf("The server got lost while finding the topics\n");
    exit(EXIT_FAILURE);
  }
  sscanf(buffer, "%d %d", &response, &chat_room);

	if(response == OK) {
		// Recieve the topics if the name creating was OK
		if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ) {
	    printf("The server got lost while finding the topics\n");
	    exit(EXIT_FAILURE);
	  }
	  sscanf(buffer, "%s %s %s", topics[0], topics[1], topics[2]);
	  // Ask user what topics does he want to join, save in chat_room
	  printf("\n-+-+-+-+- Select a topic -+-+-+-+-\n");
	  printf("\t 0. %s\n", topics[0]);
	  printf("\t 1. %s\n", topics[1]);
	  printf("\t 2. %s\n", topics[2]);
	  printf("Enter an option to start writing in the topic: ");
	  scanf("%d", &chat_room);
		getchar();
	} else {
		printf("Something went wrong creating your user. Try again");
		exit(EXIT_FAILURE);
	}
	// Send the topic selected by the user
	operation = TOPIC;
	sprintf(buffer, "%d %d", operation, chat_room);
	sendString(connection_fd, buffer);

	// We are added to the server room, we wait for the key in order to comunicate
	if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ) {
    printf("The server had problems connecting you with the room\n");
    exit(EXIT_FAILURE);
	}
	sscanf(buffer, "%d", &response);
	if(response == KEY) {
		// If the server sends the key response we star listening for the key value
		if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ) {
      printf("The server had problems with the ids\n");
      exit(EXIT_FAILURE);
	  }
	  sscanf(buffer, "%s", KEY_VAL);
	} else {
		printf("An error happend with the key\n");
		exit(EXIT_FAILURE);
	}
	// We are ready to start using the messaging system with our server and other
	// clients
	while(opt != 'e'){
		printf("\n-+-+-+-+- ChatRoom Menu -+-+-+-+-\n");
		printf("\tw. Write Message\n");
		printf("\tr. Refresh\n");
		printf("\te. Exit\n");
		printf("Enter option: ");
		scanf("%c", &opt);
		getchar();
		switch (opt) {
			case 'w':
				// The user will send a new message, we notify the server
				operation = SEND;
				sprintf(buffer, "%d 0", operation);
				sendString(connection_fd, buffer);
				// Ask user for message
				printf("Enter message:\n");
				fgets(message, BUFFER_SIZE, stdin);
				sprintf(buffer, "%s: %s", name, message);
				// Encrypt buffer using blowfish algorithm
				char *_buffer = blowfish_Encrypt(buffer, KEY_VAL);
				// Send encrypted text to server
				sendString(connection_fd, _buffer);
				free(_buffer);
				break;
			case 'r':
				// The user wants to see all the messages, we notify the server
				operation = SHOW;
				sprintf(buffer, "%d 0", operation);
				sendString(connection_fd, buffer);

				// We'll recieve the number of messages that the server will send
				if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ) {
					printf("The server disconnected\n");
					exit(EXIT_FAILURE);
				}
				sscanf(buffer, "%d %d", &response, &numOfMessages);
				printf("\n");
				// We start listening for the messages sent by the server
				for (size_t i = 0; i < numOfMessages; i++) {
					if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ) {
						printf("The server disconnected\n");
						exit(EXIT_FAILURE);
					}
					// Decrypt buffer using blowfish algorithm
					char *_buffer = blowfish_Decrypt(buffer, KEY_VAL);
					// This will print the message like +name+: +message+
					printf("\t%s\n", _buffer);
					free(_buffer);
				}
				break;
			case 'e':
				// The user wants to quit, we notify the server
				operation = EXIT;
				sprintf(buffer, "%d 0", operation);
				sendString(connection_fd, buffer);

				// Due to protocol, we listen after sending EXIT
				if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ) {
		      printf("The server had problems with the ids\n");
		      exit(EXIT_FAILURE);
			  }
				sscanf(buffer, "%d", &response);
				break;
			default:
				printf("Invalid option. Try again...\n");
		    continue;
		}
		// Finish interaction with the server, end of program
		if(response == BYE) {
			printf("Thanks for using our chat tables!\n");
			break;
		}
	}

}

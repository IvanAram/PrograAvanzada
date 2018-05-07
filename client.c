/*
    Program for a chat client
    It uses sockets, threads (paralelism), signals and __moreinfohere__

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
void chatOperations(int);

///// MAIN FUNCTION
int main(int argc, char *argv[]){
	int connection_fd;

	// Check the correct arguments
	if (argc != 3){
		usage(argv[0]);
	}

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
    Main menu with the options available to the user
*/
void chatOperations(int connection_fd) {
	char buffer[BUFFER_SIZE];
	char name[MAX_NAME_SIZE];
	char topics[3][MAX_NAME_SIZE];
	int chat_room = 4;
	char opt;
	int operation = NAME;
	char KEY_VAL[20];
	char message[BUFFER_SIZE];
	int response = OK;
	int numOfMessages;

	// Start interaction with user's client
	printf("Enter your display name: ");
	scanf("%s", name);
	sprintf(buffer, "%d %s", operation, name);
	//printf("...SENDING NAME\n");
	sendString(connection_fd, buffer);

	// Recieve response of the user creation
	//printf("RECEIVING SERVER RESPONSE...\n");
	if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ) {
    printf("The server got lost while finding the topics\n");
    exit(EXIT_FAILURE);
  }
	//printf("BUFFER: %s\n", buffer);
  sscanf(buffer, "%d %d", &response, &chat_room);
	//printf("RESPONSE: %d\nTOPICS: %d\n", response, chat_room);
	if(response == OK) {
	// Recieve the topics if the name creating was OK
		//printf("RECEIVING TOPICS...\n");
		if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ) {
	    printf("The server got lost while finding the topics\n");
	    exit(EXIT_FAILURE);
	  }
		//printf("BUFFER: %s\n", buffer);
	  sscanf(buffer, "%s %s %s", topics[0], topics[1], topics[2]);
		//printf("TOPICS: %s %s %s\n", topics[0], topics[1], topics[2]);
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
	operation = TOPIC;
	sprintf(buffer, "%d %d", operation, chat_room);
	//printf("...SENDING OPERATION\n");
	sendString(connection_fd, buffer);

	//printf("RECEIVING RESPONSE...\n");
	if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ) {
    printf("The server had problems connecting you with the room\n");
    exit(EXIT_FAILURE);
	}
	//printf("BUFFER: %s\n", buffer);
	sscanf(buffer, "%d", &response);
	//printf("RESPONSE: %d\n", response);
	if(response == KEY) {
		//printf("RECEIVING KEY...\n");
		if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ) {
      printf("The server had problems with the ids\n");
      exit(EXIT_FAILURE);
	  }
		//printf("BUFFER: %s\n", buffer);
	  sscanf(buffer, "%s", KEY_VAL);
		//printf("KEY: %s\n", KEY_VAL);
	} else {
		printf("An error happend with the key\n");
		exit(EXIT_FAILURE);
	}

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
				operation = SEND;
				sprintf(buffer, "%d 0", operation);
				//printf("...SENDING OPERATION\n");
				sendString(connection_fd, buffer);

				printf("Enter message:\n");
				fgets(message, BUFFER_SIZE, stdin);

				sprintf(buffer, "%s:%s", name, message);

				//scanf("%s", message);
				//getchar();

				// ENCRYPT MESSAGE

				//printf("...SENDING MESSAGE\n");
				sendString(connection_fd, buffer);
				break;
			case 'r':
				operation = SHOW;
				sprintf(buffer, "%d 0", operation);
				//printf("...SENDING OPERATION\n");
				sendString(connection_fd, buffer);

				//printf("RECEIVING RESPONSE...\n");
				if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ) {
					printf("The server had problems with the ids\n");
					exit(EXIT_FAILURE);
				}
				//printf("BUFFER: %s\n", buffer);
				sscanf(buffer, "%d %d", &response, &numOfMessages);
				//printf("RESPONSE: %d\nMESSAGES: %d\n", response, numOfMessages);

				for (size_t i = 0; i < numOfMessages; i++) {
					//printf("RECEIVING MESSAGE...\n");
					if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ) {
						printf("The server had problems with the ids\n");
						exit(EXIT_FAILURE);
					}
					//printf("BUFFER: %s\n", buffer);
					// BUFFER IS MESSAGE (ENCRIPTED) DECRYPT MSG

					printf("\t%s\n", buffer);
				}
				break;
			case 'e':
				operation = EXIT;
				sprintf(buffer, "%d 0", operation);
				//printf("...SENDING OPERATION\n");
				sendString(connection_fd, buffer);

				//printf("RECEIVING RESPONSE...\n");
				if ( !recvString(connection_fd, buffer, BUFFER_SIZE) ) {
		      printf("The server had problems with the ids\n");
		      exit(EXIT_FAILURE);
			  }
				//printf("BUFFER: %s\n", buffer);
				sscanf(buffer, "%d", &response);
				break;
			default:
				printf("Invalid option. Try again...\n");
		    continue;
		}
		if(response == BYE) {
			printf("Thanks for using our chat tables!\n");
			break;
		}
	}

}

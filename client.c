/*
    Program for a chat client
    It uses sockets, threads (paralelism), signals and __moreinfohere__

    Ivan Aram Gonzalez Su - A01022584
    Jose Manuel Beauregard Mendez - A01021716
    Sebastian Galguera Ortega - A01016708
*/

/*
    Client program to access the accounts in the bank
    This program connects to the server using sockets

    Gilberto Echeverria
    gilecheverria@yahoo.com
    29/03/2018
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

#define BUFFER_SIZE 1024

///// FUNCTION DECLARATIONS
void usage(char *);
void chatOperations(int);

///// MAIN FUNCTION
int main(int argc, char *argv[]){
  int connection_fd;

  // Check the correct arguments
  if (argc != 3)
    usage(argv[0]);


  // Start the server
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
void chatOperations(int connection_fd){

}

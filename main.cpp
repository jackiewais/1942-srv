#include <iostream>
#include <sys/socket.h>
#include "SDL/SDL_thread.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

int cantCon;


static int doProcessing (void *sockfd) {
   int n;
   bool finish = false;
   char buffer[256];

   int sock = *(int*)sockfd;

   //Receive a message from client
     while(!finish)
     {
    	 //Read client's message
    	 	bzero(buffer,256);
    	 	n = read(sock,buffer,255 );
    	 	if (n < 0) {
    	 	  perror("ERROR reading from socket \n");
    	 	  //exit(1);
    	 	  finish = true;
    	 	}

    	 	if (n == 0){
    	 		//Exit message
    	 		printf("Exit message received \n");
    	 		finish = true;
    	 		cantCon--;
    	 	}else{

				printf("Here is the message: %s \n",buffer);

				//Respond to the client
				n = write(sock,"I got your message \n",21);
				if (n < 0) {
				  perror("ERROR writing to socket \n");
				  exit(1);
				 }
    	 	}
     }

	free(sockfd);
	close(sock);
	return 0;
}


int main()
{
    cout << "Starting...\n";

	int sockfd, newsockfd, portno, *new_sock;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	int maxCon;
	cantCon = 0;

	SDL_Thread *thread;

	//Parameter definition
	portno = 5001;
	maxCon = 2;

	// Opening the Socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
	 perror("ERROR opening socket");
	 exit(1);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	// Binding server address with socket
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	  perror("ERROR on binding");
	  exit(1);
	}

	//Listen through the socket
	listen(sockfd,5);
	clilen = sizeof(cli_addr);

	 while (1) {
		 cout << "CantCon: " << cantCon << " \n";

		 //Accept connection from client
		 newsockfd = accept(sockfd, (sockaddr *) &cli_addr, &clilen);
		 if (newsockfd < 0) {
			perror("ERROR on accept");
			exit(1);
		}

	    if (cantCon == maxCon)
	    {
	    	char* message = "ERROR: The Server has exceeded max number of connections. \n";
	    	write(newsockfd,message,strlen(message));
	        close(newsockfd);
	    }else{
	    	cantCon++;

	    	new_sock = (int *)malloc(sizeof(int));
	    	*new_sock = newsockfd;

	    	thread = SDL_CreateThread(doProcessing, (void *)new_sock);

	        if (NULL == thread) {
	            printf("\nSDL_CreateThread failed: %s", SDL_GetError());
	        }
	        /*else {
	            //SDL_WaitThread(thread, &threadReturnValue);
		    	//cantCon--;
	            //printf("\nThread returned value: %d", threadReturnValue);
	        }*/
	    }
	}

    return 1;
}

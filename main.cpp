#include <iostream>
#include <map>
#include <sys/socket.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include<sys/errno.h>

#include "SDL2/SDL_thread.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

int cant_con;
map<int,int> socket_queue;

struct msg_buf {
  long mtype;
  char mtext[200];
};

extern int errno;       // error NO.

int writeQueueMessage(int socket, int msgid, char* message){
	msg_buf msg;
	int msgqid, rc;

	// message to send
	msg.mtype = msgid; // set the type of message
	sprintf (msg.mtext, "%s\n", message); /* setting the right time format by means of ctime() */

	msgqid = socket_queue[socket];
	// send the message to queue
	rc = msgsnd(msgqid, &msg, sizeof(msg.mtext), 0); // the last param can be: 0, IPC_NOWAIT, MSG_NOERROR, or IPC_NOWAIT|MSG_NOERROR.
	if (rc < 0) {
		perror( strerror(errno) );
		printf("msgsnd failed, rc = %d\n", rc);
	}

	return rc;

}
static int doReading (void *sockfd) {
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
    	 	  finish = true;
    	 	}

    	 	if (n == 0){
    	 		//Exit message
    	 		printf("Exit message received \n");
				writeQueueMessage(sock,99, "Exit message");
    	 		finish = true;
    	 		cant_con--;
    	 	}else{
				printf("Here is the message: %s \n",buffer);

				//Respond to the client
				n = write(sock,"I got your message \n",21);
				if (n < 0) {
				  perror("ERROR writing to socket \n");
				  finish = true;
				 }

				writeQueueMessage(sock,1, buffer);
    	 	}
     }

	free(sockfd);
	close(sock);
	return 0;
}

static int doWriting (void *sockfd) {
   int n, rc;
   bool finish = false;
   msg_buf msg;

   int sock = *(int*)sockfd;

   int msgqid;

   //Receive a message from client
     while(!finish)
     {
		msgqid = socket_queue[sock];
		rc = msgrcv(msgqid, &msg, sizeof(msg.mtext), 0, 0);
		if (rc < 0) {
			perror( strerror(errno) );
			printf("msgrcv failed, rc=%d\n", rc);
			return 1;
		}

		if (msg.mtype == 99){
			//Exit message received
			printf("Exit message received: %s\n", msg.mtext);
			finish = true;
		}else{
			printf("received msg: %s\n", msg.mtext);
			//Respond to the client
			n = write(sock,"I got your queue message \n",26);
			if (n < 0) {
			  perror("ERROR writing to socket \n");
			  finish = true;
			}
		}
	}
	return 0;
}

void createAndDetachThread(SDL_ThreadFunction fn, const char *name, void *data){
	SDL_Thread *thread;

	thread = SDL_CreateThread(fn,name, data);
	if (NULL == thread) {
		printf("\nSDL_CreateThread failed: %s", SDL_GetError());
	}
	SDL_DetachThread(thread);

}
int manageNewConnection(int newsockfd){

	int *newsock_dir;
	int msgqid;

	//Create the output messages queue for the socket
	msgqid = msgget(IPC_PRIVATE, 0666|IPC_CREAT|IPC_EXCL);
   if (msgqid < 0) {
	  perror(strerror(errno));
	  printf("failed to create message queue with msgqid = %d\n", msgqid);
	  return 1;
	}
	socket_queue[newsockfd] = msgqid;

	newsock_dir = (int *)malloc(sizeof(int));
	*newsock_dir = newsockfd;

	createAndDetachThread(doReading,"doReading", newsock_dir);
	createAndDetachThread(doWriting,"doWriting", newsock_dir);
}

int openAndBindSocket(int port_number){
	int sockfd;
	struct sockaddr_in serv_addr;

	// Opening the Socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
	 perror("ERROR opening socket");
	 exit(1);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port_number);

	// Binding server address with socket
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	  perror("ERROR on binding");
	  exit(1);
	}

	//Listen through the socket
	listen(sockfd,5);

	return sockfd;
}

int main()
{
    cout << "Starting...\n";

	int sockfd, newsockfd, port_number, max_con;
	socklen_t cli_len;
	struct sockaddr_in serv_addr, cli_addr;

	cant_con = 0;

	//Parameter definition
	port_number = 5001;
	max_con = 2;

	sockfd = openAndBindSocket(port_number);

	cli_len = sizeof(cli_addr);

	 while (1) {
		 cout << "CantCon: " << cant_con << " \n";

		 //Accept connection from client
		 newsockfd = accept(sockfd, (sockaddr *) &cli_addr, &cli_len);
		 if (newsockfd < 0) {
			perror("ERROR on accept");
		 }else{

			if (cant_con == max_con){
				char* message = "ERROR: The Server has exceeded max number of connections. \n";
				write(newsockfd,message,strlen(message));
				close(newsockfd);
			}else{
				cant_con++;
				manageNewConnection(newsockfd);
			}
		}
	}

    return 1;
}

#include <iostream>
#include <map>

#include <sys/socket.h>

#include "SDL2/SDL_thread.h"

#include <string.h>
#include <stdio.h>

#include <netinet/in.h>
#include <unistd.h>

#include "Logger/Log.h"
#include "Utils/queueManager.h"

using namespace std;

using namespace queueManager;
SDL_mutex *mutexQueue;

int cant_con, input_queue;
map<int,int> socket_queue;
Log slog;

bool writeQueueMessage(int socket, int msgid, char* message, bool socketQueue){
//When socketQueue = true writes on the socket queue. If its false, writes on the input queue

	msg_buf msg;
	int msgqid;

	// message to send
	msg.mtype = msgid;
	msg.minfo.msocket = socket;
	sprintf (msg.minfo.mtext, "%s", message);

	msgqid = (socketQueue) ? socket_queue[socket] : input_queue;

	return sendQueueMessage(msgqid,msg);
}

static int processMessages (void *data) {
	msg_buf msg;
	bool finish = false;

	if (!getQueue(input_queue)) {
		return 1;
	}

	while(!finish){
		if (!receiveQueueMessage(input_queue, msg)) {
			return 1;
		}else{
			//Validate Message
			//Process Message

			slog.writeLine("processMessages | Processing input queue message: " + string(msg.minfo.mtext));
			//Writes the message in the socket's queue
			writeQueueMessage(msg.minfo.msocket,msg.mtype, msg.minfo.mtext, true);
		}
	}

	return 0;

}

static int doReading (void *sockfd) {
   int n;
   bool finish = false;
   char buffer[256];
   mutexQueue = SDL_CreateMutex();
   int sock = *(int*)sockfd;

   //Receive a message from client
   while(!finish){
	   //Read client's message
		bzero(buffer,256);
		n = recv(sock,buffer,255,0 );
		if (n < 0) {
			slog.writeErrorLine("doReading | ERROR reading from socket");
			finish = true;
		}else if (n == 0){
			//Exit message
			slog.writeLine("doReading | Exit message received");
			writeQueueMessage(sock,99, (char*) "Client Connection Closed", true);
			finish = true;
		}else{
			slog.writeLine("doReading | Client send this message: " + string(buffer));
			//Respond OK to the client
			n = send(sock,"I got your message \n",21,0);
			if (n < 0) {
				slog.writeErrorLine("doReading | ERROR writing to socket");
				finish = true;
			}

			if (!finish){
				//Puts the message in the input queue
				//mutex lock.
				if (SDL_LockMutex(mutexQueue) == 0) {
					slog.writeLine("doReading | Inserting message '" + string(buffer) + "' into clients queue");
					finish = !writeQueueMessage(sock,1, buffer,false);
				//mutex unlock
				    SDL_UnlockMutex(mutexQueue);
				}
				    SDL_DestroyMutex(mutexQueue);
			}
		}
     }

   	cant_con--;
	free(sockfd);
	close(sock);
	return 0;
}

static int doWriting (void *sockfd) {
   int msgqid, n;
   bool finish = false;
   msg_buf msg;

   int sock = *(int*)sockfd;

   //Receive a message from client
     while(!finish)
     {
		msgqid = socket_queue[sock];
		if (!receiveQueueMessage(msgqid, msg)) {
			return 1;
		}

		if (msg.mtype == 99){
			//Exit message received
			slog.writeLine("doWriting | Exit message received: " + string(msg.minfo.mtext));
			socket_queue.erase(sock);
			finish = true;
		}else{
			slog.writeLine("doWriting | Received queue message: " + string(msg.minfo.mtext));
			//Respond to the client
			n = send(sock,"I processed your message \n",26,0);
			if (n < 0) {
			  slog.writeErrorLine("doWriting | ERROR writing to socket");
			  finish = true;
			}
		}
	}

 	free(sockfd);
	return 0;
}

void createAndDetachThread(SDL_ThreadFunction fn, const char *name, void *data){
	SDL_Thread *thread;

	thread = SDL_CreateThread(fn,name, data);
	if (NULL == thread) {
		slog.writeLine("SDL_CreateThread failed: " +  string(SDL_GetError()) + "");
	}
	SDL_DetachThread(thread);

}

void manageNewConnection(int newsockfd){

	int *newsock_dir;
	int msgqid;

	//Create the output messages queue for the socket
	if (getQueue(msgqid)) {
		socket_queue[newsockfd] = msgqid;

		newsock_dir = (int *)malloc(sizeof(int));
		*newsock_dir = newsockfd;

		createAndDetachThread(doReading,"doReading", newsock_dir);
		createAndDetachThread(doWriting,"doWriting", newsock_dir);
	}
}

int openAndBindSocket(int port_number){
	int sockfd;
	struct sockaddr_in serv_addr;

	// Opening the Socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		slog.writeErrorLine("openAndBindSocket | ERROR opening socket");
		slog.writeLine("Application closed.");
		exit(1);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port_number);

	// Binding server address with socket
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	  slog.writeErrorLine("openAndBindSocket | ERROR on binding");
	  slog.writeLine("Application closed.");
	  exit(1);
	}

	//Listen through the socket
	listen(sockfd,5);

	return sockfd;
}


static int exitManager (void *data) {
	  string input;
	  bool quit = false;
	  int n;

	  cout << "Type 'quit' any time to exit \n";
	  while (!quit){
		  getline (cin, input);
		  quit = (input == "quit");
	  }

	  slog.writeLine("Exit signal received. Closing application...");

	  for(auto const &it : socket_queue) {
		  n = send(it.first,"Server is closed \n",21,0);
		  if (n < 0) {
			  slog.writeErrorLine("exitManager | ERROR writing to socket");
		  }
		  close(it.first);
		  cant_con --;
	  }

	  slog.writeLine("Application closed.");

	  exit(0);
	  return 0;
}

int main(int argc, char **argv)
{
	int sockfd, newsockfd, port_number, max_con;
	socklen_t cli_len;
	struct sockaddr_in cli_addr;

	cant_con = 0;

	//Parameter definition
	port_number = 5001;
	max_con = 2;

	//Log initialize
	slog.createFile();

	slog.writeLine("Starting...");
	createAndDetachThread(exitManager,"exitManager", NULL);

	sockfd = openAndBindSocket(port_number);

	createAndDetachThread(processMessages,"processMessages", NULL);

	cli_len = sizeof(cli_addr);

	 while (1) {

		 cout << "Current connections: " << cant_con << " \n";

		 //Accept connection from client
		 newsockfd = accept(sockfd, (sockaddr *) &cli_addr, &cli_len);
		 if (newsockfd < 0) {
			slog.writeErrorLine("ERROR on accept");
		 }else{

			if (cant_con == max_con){
				send(newsockfd,"ERROR: The Server has exceeded max number of connections. \n",60,0);
				close(newsockfd);
			}else{
				cant_con++;
				manageNewConnection(newsockfd);
			}
		}
	}

    return 1;
}

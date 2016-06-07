#include <iostream>
#include <map>

#include <sys/socket.h>

#include "SDL2/SDL_thread.h"

#include <string>
#include <stdio.h>
#include <limits>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>
#include "Elemento/Elemento.h"
#include "Logger/Log.h"
#include "Utils/queueManager.h"
#include "Utils/messages.h"
#include "Parser/Parser.h"

#define BUFLEN 1000

using namespace std;
using namespace Parser;
using namespace queueManager;

SDL_mutex *mutexQueue;
SDL_mutex *mutexCantClientes;
SDL_mutex *mutexClientState;
SDL_mutex *mutexProgress;
int cant_con, input_queue, cantJug, audit;
map<int,int> socket_queue;
map<int,bool> client_state;
map<int,Elemento*> elementos;
map<string,int> users;
unsigned int *progress;
Log slog;
bool quit = false;
bool readyToStart = false;
bool playing = false;
type_Ventana ventana;
list<type_Sprite> sprites;
type_Escenario escenario;
type_Avion jugador;
string pathXMLEscenario = "escenario.xml";

map<int,int> socket_to_id;
//====================================================================================================


bool writeQueueMessage(int socket, struct gst** messages, int msgQty, bool socketQueue){
//When socketQueue = true writes on the socket queue. If its false, writes on the input queue
	msg_buf msg;
	int msgqid;

	// message to send
	msg.msocket = socket;
	msg.minfo = messages;
	msg.msgQty = msgQty;
	
	msgqid = (socketQueue) ? socket_queue[socket] : input_queue;


	return sendQueueMessage(msgqid,msg);

}

bool check_integer(const char* value) {
  for(unsigned int i = 0; i < strlen(value); ++i) {
    if(!isdigit(value[i])){
	if((value[i]!='-')or (i!=0)){
	return false;
		}	
	}
      
  }
  return true;
}

bool check_double(const char* value) {
	char *p;
	strtod(value, & p );
	return (* p == 0 );
}

bool check_char(const char* value) {
	return strlen(value) == 1;
}

void leerXMLEscenario() {
	type_DatosGraficos* xml;

	xml = parseXMLServerMap(pathXMLEscenario.c_str(), &slog);

	ventana = xml -> ventana;
	sprites = xml -> sprites;
	escenario = xml -> escenario;
	jugador = xml -> avion;
	cantJug = xml -> cantJug;
	slog.writeLine("Cantidad de jugadores: " + to_string(cantJug));
}

void resetProgress(){
	if (SDL_LockMutex(mutexProgress) == 0) {
		for (int i = 0; i < cantJug; i++ ){
			progress[i] = 1;
		}
	SDL_UnlockMutex(mutexProgress);
	}
}

void addProgress(int id){
	if (SDL_LockMutex(mutexProgress) == 0) {
		progress[id -1]++;
		//cout << "DEBUG: addProgress progress[" << id << "] = " << progress[id-1] << endl;
	SDL_UnlockMutex(mutexProgress);
	}
}

void updateProgress(int id, int prog){
	if (SDL_LockMutex(mutexProgress) == 0) {
		progress[id -1] = prog;
	SDL_UnlockMutex(mutexProgress);
	}
}

int getProgress(){
	int temp = 1;
	if (SDL_LockMutex(mutexProgress) == 0) {
		for (int i = 0; i < cantJug; i++ ){
			if (progress[i] > temp){
				temp = progress[i];
			}
		}
	SDL_UnlockMutex(mutexProgress);
	}
	return temp;
}

void buscarPathSprite(Parser::spriteType idSprite, char*& path) {
	list<type_Sprite>::iterator it;
	bool encontrado = false;
	it = sprites.begin();
	while (!encontrado && (it != sprites.end())){
		encontrado = (it)->id == idSprite;
		if (encontrado){
			//cout << "DEBUG buscarParhSprite (it)->path: " << (it)->path << endl;
			encontrado = true;
			//memset(path, '\0', pathl);
			//memcpy(path, it -> path, pathl);
			path = (it)->path;
			//cout << "DEBUG buscarParhSprite path: " << path << endl;
		}
		it++;
	}
	if (!encontrado){
		//cout << "DEBUG buscarParhSprite: path no encontrado" << endl;
		path[0] = '-';
	}
}

int _processMsgs(struct gst** msgs, int socket, int msgQty, struct gst*** answerMsgs){

	//cout << "DEBUG entro a _processMsgs" << endl;
	//cout << "DEBUG _processMessages in" << endl;

	int answerMsgsQty = 0, tempId;
	Elemento* tempEl;
	bool newEvent = false, oldEvent = false, isEscenario = false, isSprites = false;
	status event, prevStatus, tempSta;

	for (int i = 0; i < msgQty; i++){

		if (msgs[i] -> type[0] == '2'){
			tempId = atoi(msgs[i]-> id);
			tempEl = elementos[tempId];
			//cout << "DEBUG _processMsgs esta queriendo actualizar" << endl;
			if (tempEl == NULL){
				cout << "recibido elemento inexistente " << endl;
				cout << "id = " << tempId << endl;
			}else {
				tempSta = (status) msgs[i] -> info[0];
				if ((tempSta == status::START) ||
					(tempSta == status::RESET) ||
					(tempSta == status::PAUSA) ||
					(tempSta == status::NO_PAUSA)){

					//cout << "_processMsgs DEBUG nuevoEvento = " << msgs[i] -> info[0] << endl;
					if(readyToStart){

						if ((tempSta == status::START) && !playing){
							resetProgress();
							playing = true;
						}
						else if ((tempSta == status::PAUSA) || (tempSta == status::NO_PAUSA)){
							playing = !playing;
						}
						else{		//tempSta == RESET

							//cout << "DEBUG _processMsgs llego un reset " << endl << endl;
							resetProgress();
							playing = true;
						}

						newEvent = true;
						event = tempSta;

						prevStatus = tempEl -> getEstado(tempId);
						tempEl-> update(msgs[i]);
						tempEl-> updateStatus(prevStatus);

					}
				}

				else {
					tempSta = tempEl -> getEstado(tempId);
					if ((tempSta == status::START) ||
						(tempSta == status::RESET) ||
						(tempSta == status::PAUSA) ||
						(tempSta == status::NO_PAUSA)){

					//cout << "_processMsgs DEBUG viejoEvento = " << (char)tempEl -> getEstado() << endl;
					oldEvent = true;
					event = tempSta;
					tempEl-> update(msgs[i]);

					}
					else{
					//cout << "_processMsgs DEBUG info = " << msgs[i] -> info[0] << endl;
					tempEl-> update(msgs[i]);
					}
					//cout << "DEBUG _processMsgs actualizo el elemento" << endl;
				}

				if (playing){
					addProgress(tempId);
				}

			}
		}
		else if (msgs[i] -> type[0] == '8'){
			if (msgs[i] -> info[0] == (char) command::DISCONNECT){
				//cout << "DEBUG _processMsgs llego disconnect" << endl;
				if (SDL_LockMutex(mutexClientState) == 0) {
					client_state[socket] = false;
					SDL_UnlockMutex(mutexClientState);
				}
			}
			if (msgs[i] -> info[0] == (char) command::REQ_SCENARIO) {
				leerXMLEscenario();
				isEscenario = true;
			}
			if (msgs[i] -> info[0] == (char) command::REQ_SPRITES) {
				isSprites = true;
			}
		}
		delete msgs[i];
	}
	delete[] msgs;

	if (isSprites) {
		int i = 0;
		answerMsgsQty = sprites.size();
		*answerMsgs = new struct gst*[answerMsgsQty];
		struct gst** answerIt = (*answerMsgs);
		list<type_Sprite>::iterator iter;
		for (iter = sprites.begin(); iter != sprites.end(); iter ++) {
			answerIt[i] = genGstFromSprite(&(*iter));
			i++;
		}
	

	} else if (isEscenario) {
		char *pathFondo, *pathElemento;
		//memset(pathFondo, '=', pathl);
		//memset(pathElemento, '=', pathl);
		int i = 4;
		buscarPathSprite(escenario.fondo.spriteId, pathFondo);
		//cout << "DEBUG _processMsgs pathFondo: " << pathFondo << endl;
		answerMsgsQty = escenario.elementos.size() + 4;
		*answerMsgs = new struct gst*[answerMsgsQty];
		struct gst** answerIt = (*answerMsgs);
		answerIt[0] = genGstFromCantJug(cantJug);
		answerIt[1] = genGstFromVelocidades(jugador.velocidadDesplazamiento, jugador.velocidadDisparos);
		answerIt[2] = genGstFromFondo(&escenario, pathFondo);
		answerIt[3] = genGstFromVentana(&ventana);
		list<type_Elemento>::iterator ite;
		for (ite = escenario.elementos.begin(); ite != escenario.elementos.end(); ite ++) {
			buscarPathSprite((ite) -> spriteId, pathElemento);
			//cout << "DEBUG _processMsgs pathElemento: " << pathElemento << endl;
			answerIt[i] = genGstFromElemento(&(*ite), pathElemento);
			i++;
		}
	//} else {
	} else if (client_state[socket]){
		//cout << "DEBUG _processMsgs llego al else" << endl;
		answerMsgsQty = elementos.size();
		*answerMsgs = new struct gst*[answerMsgsQty];
		struct gst** answerIt = (*answerMsgs);
		map<int,Elemento*>::iterator elementosIt;
		elementosIt = elementos.begin();
		for (int i = 0; i < answerMsgsQty; i++){
	
			answerIt[i] = genUpdateGstFromElemento(elementosIt -> second, tempId);
	
			if ((newEvent || oldEvent) && (elementosIt -> first == tempId)){
				answerIt[i] -> info[0] = (char) event;
			}
			else if (newEvent){
				if (elementosIt -> second -> getEstado(tempId) != status::DESCONECTADO){
					//cout << "DEBUG _processMsgs: entro a !desconectado " << elementosIt -> first <<  endl;
					elementosIt -> second -> updateStatus(event);
				}
				else if (tempSta == status::RESET){
					//cout << "DEBUG _processMsgs: entro a resetear pos" <<  endl;
					int posx = ventana.ancho * (elementosIt -> second -> getId())/ (cantJug + 1);
					int posy = ventana.alto - 68;
					elementosIt -> second -> updatePos(posx, posy);
				}
			}
		
			elementosIt++;
		}
	}
	//cout << "DEBUG _processMessages out" << endl;
	return answerMsgsQty;

}


static int processMessages (void *data) {
	msg_buf msg;

	bool finish = false;
	
	if (!getQueue(input_queue)) {
		return 1;
	}

	struct gst** answerMsgs;
	int msgQty;

	while(!finish){
		//cout << "DEBUG processMessages in" << endl;
		if (!receiveQueueMessage(input_queue, msg)) {
			return 1;
		}else{

			msgQty = _processMsgs(msg.minfo,msg.msocket, msg.msgQty, &answerMsgs);

			//Writes the message in the socket's queue
			writeQueueMessage(msg.msocket, answerMsgs, msgQty, true);
						
			//cout << "DEBUG processMessages se escribio en la queue de escritura" << endl;
		}
		//cout << "DEBUG processMessages out" << endl;

	}
	
	return 0;

}


bool doReadingError(int n, int sock, char* buffer){

	//cout << "DEBUG doReadingError in" << endl;
	struct gst* msgExit = genAdminGst(0, command::DISCONNECT);
	bool finish=false;

	if (n < 0) {
		slog.writeErrorLine("doReadingError | ERROR reading from socket");
		writeQueueMessage(sock, &msgExit, 1, true);
		finish = true;

		if (SDL_LockMutex(mutexClientState) == 0) {
			client_state[sock] = false;
		    SDL_UnlockMutex(mutexClientState);
		}
	elementos[socket_to_id[sock]]-> updateStatus(status::DESCONECTADO);
	}

	else if (n == 0){
		//Exit message
		slog.writeLine("doReadingError | Exit message received");
		writeQueueMessage(sock, &msgExit, 1, true);
		finish = true;

		if (SDL_LockMutex(mutexClientState) == 0) {
			client_state[sock] = false;
			SDL_UnlockMutex(mutexClientState);
		}
	}
	//cout << "DEBUG doReadingError out" << endl;
	return finish;

}

//put the message into mainqueuemessages ( with lock & unlock mutex)
bool insertingMessageQueue(int sock, char *buffer){
	struct gst** msgs;
	int msgQty;
	bool finish = false;


	//mutex lock.
	if (SDL_LockMutex(mutexQueue) == 0) {
		//slog.writeLine("insertingMessageQueue | Inserting message '" + string(buffer) + "' into clients queue");
		msgQty = decodeMessages(&msgs, buffer);
		finish = !writeQueueMessage(sock, msgs, msgQty, false);
	 //mutex unlock
	 SDL_UnlockMutex(mutexQueue);
	}

	return finish;
}



static int doReading (void *sockfd) {
   int n;
   bool finish = false;
   char buffer[BUFLEN];

   int sock = *(int*) sockfd;
   free(sockfd);
   char messageLength[3];
   int messageSize;
   //char bufferingMessage[BUFLEN];

   //Receive a message from client
   while(!finish && !quit){
	   //cout << "DEBUG doReading in" << endl;
       //cout << "DEBUG doReading sock = " << sock << endl;
       //Read client's message
        //bzero(bufferingMessage,BUFLEN);
        bzero(buffer,BUFLEN);
        n = recv(sock, buffer, BUFLEN-1, 0);
           //cout << "DEBUG DOREADING n = " << n << endl;
        if (audit)
            cout << "AUDIT rcv: " << buffer << endl;
        //handle Error reading
        finish = doReadingError(n, sock, buffer);

        if (finish){
        	//cout << "DEBUG doReading finish = true" << endl;
            if (SDL_LockMutex(mutexClientState) == 0) {
                client_state[sock] = false;
                SDL_UnlockMutex(mutexClientState);
            }

        }
        else{
        	//cout << "DEBUG doReading finish = false" << endl;
            //getting the messageLength
            strncpy(messageLength,buffer,3);
            char* bufferIt = buffer;
            /*while (!(messageLength[0] >= '0' && messageLength[0] <= '9') &&
                   !(messageLength[1] >= '0' && messageLength[1] <= '9') &&
                   !(messageLength[2] >= '0' && messageLength[2] <= '9') &&
                   ((n-3) > 0)){
                bufferIt++;
                n--;
                strncpy(messageLength,bufferIt,3);
                //cout << "DEBUG DOREADING buffer = " << bufferIt << endl;
            }*/

            messageSize = atoi(messageLength);

            if (n>=messageSize){//full message received.
                finish=insertingMessageQueue(sock,bufferIt);
            }else{//message incomplete.
                int readed=n;
                cout << messageSize << endl;
                while ( readed !=messageSize){
                    n =recv(sock,bufferIt+readed,messageSize-readed,0);
                    cout << "loopeando dentro del doreading" << endl;
                    readed+=n;
                }
                finish=insertingMessageQueue(sock,bufferIt);
             }
            if (SDL_LockMutex(mutexClientState) == 0) {
                finish = !client_state[sock];
                SDL_UnlockMutex(mutexClientState);
                //cout << "DEBUG doReading finish = " << finish << endl;
            }
        }
        //cout << "DEBUG doReading out" << endl;

       } // END WHILE
    if (!quit){
    if (SDL_LockMutex(mutexCantClientes) == 0) {
         cant_con--;
      SDL_UnlockMutex(mutexCantClientes);
    }

    close(sock);
}
    return 0;
}

static int doWriting (void *sockfd) {
   int msgqid, n;
   bool finish = false;
   msg_buf msg;

   int sock = *(int*) sockfd;

   bool* isSocketActive;
   if (SDL_LockMutex(mutexClientState) == 0) {
	   isSocketActive = &client_state[sock];
	   SDL_UnlockMutex(mutexClientState);
   }
   //cout << "DEBUG doWriting sock = " << sock << endl;
   free(sockfd);

   //Receive a message from client
     while(!finish && !quit)
     {
    	 //cout << "DEBUG doWriting in" << endl;
		msgqid = socket_queue[sock];
		if (!receiveQueueMessage(msgqid, msg)) {
			return 1;
		}
		//cout << "DEBUG doWriting intentando leer isSocketActive" << endl;
		if (!*isSocketActive){
			//Exit message received
			//slog.writeLine("doWriting | Exit message received: " + string(msg.minfo.data));
			//cout << "DEBUG doWriting entro a borrar socket" << endl;
			socket_queue.erase(sock);
			finish = true;
			//cout << "DEBUG doWriting borro el socket" << endl;
		}else{
			//cout << "DEBUG doWriting entro en el else" << endl;
			//slog.writeLine("doWriting | Received queue message: " + string(msg.minfo.data));
			//Respond to the client
			char* message;
			//cout << "DEBUG doWriting esta queriendo encodear los mensajes" << endl;
			int messageLen = encodeMessages(&message, msg.minfo, msg.msgQty);
			//cout << "DEBUG doWriting encodeo los mensajes" << endl;
			/*
			for (int i = 0; i < msg.msgQty; i++){
				delete msg.minfo[i];
			}
			delete[] msg.minfo;
			 */
			if(!quit){
				//cout << "DEBUG doWriting intentando hacer send" << endl;
				n = send(sock,message,messageLen,0);
				//cout << "DEBUG doWriting superado el send" << endl;
				if (audit)
					cout << "AUDIT snd: " << message << endl;
				if (n < 0) {
					slog.writeErrorLine("doWriting | ERROR writing to socket");
					finish = true;
				}
				delete message;
			}

		}
		//cout << "DEBUG doWriting out" << endl;
     }

     return 0;
}

void createAndDetachThread(SDL_ThreadFunction fn, const char *name, int data){
	SDL_Thread *thread;
	int *data_dir;

	data_dir = (int *)malloc(sizeof(int));
	*data_dir = data;

	thread = SDL_CreateThread(fn,name, data_dir);
	if (NULL == thread) {
		slog.writeLine("SDL_CreateThread failed: " +  string(SDL_GetError()) + "");
	}
	SDL_DetachThread(thread);

}

void manageNewConnection(int newsockfd){

	int msgqid;
	//usleep(1000*1000);

	//Create the output messages queue for the socket
	if (getQueue(msgqid)) {
		socket_queue[newsockfd] = msgqid;
		createAndDetachThread(doReading,"doReading", newsockfd);
		createAndDetachThread(doWriting,"doWriting", newsockfd);
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

	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		slog.writeErrorLine("openAndBindSocket | ERROR configuring socket as reusable");

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
	  //bool quit = false;

	  cout << "Type 'quit' any time to exit \n";
	  while (!quit){
		  getline (cin, input);
		  quit = (input == "quit");
	  }

	  slog.writeLine("Exit signal received. Closing application...");
	  SDL_LockMutex(mutexCantClientes);
	  SDL_LockMutex(mutexQueue);
	  SDL_LockMutex(mutexClientState);
	  SDL_LockMutex(mutexProgress);

	  for(auto const &it : socket_queue) {
		  close(it.first);
		  cant_con --;
	  }
	 

      SDL_DestroyMutex(mutexQueue);
      SDL_DestroyMutex(mutexCantClientes);
      SDL_DestroyMutex(mutexClientState);
      SDL_DestroyMutex(mutexProgress);
	  slog.writeLine("Application closed.");

	  exit(0);
	  return 0;
}

void leerXML(int &cantMaxClientes, int &puerto, int &audit){

	type_datosServer xml;
	string path;

	cout << "Please enter the XML config path, or 'default' to use default file.:" << endl;
	cin >> path;
	if (path == "default"){
		slog.writeWarningLine("Se toma el XML de default");
		path = getDefaultNameServer();
	}

	while (!fileExists(path.c_str())){
		cout << "Invalid path. Pease enter the XML config path, or 'default' to use default file." << endl;
		cin >> path;
		if (path == "default"){
			slog.writeWarningLine("Se toma el XML de default");
			path = getDefaultNameServer();
		}
	}

	xml = parseXMLServer(path.c_str(), &slog);

	cantMaxClientes = xml.cantMaxClientes;
	puerto = xml.puerto;
	audit = xml.audit;
}



Elemento* genNewPlayer(int playerId, string username){
	int anchoPantalla = ventana.ancho / (cantJug + 1);
	int altura = ventana.alto - 68;
	users[username] = playerId;
	Elemento* newPlayer = new Elemento(playerId, anchoPantalla * playerId, altura, cantJug);
	elementos[playerId] = newPlayer;

	return newPlayer;
}


int main(int argc, char **argv)
{
	int sockfd, newsockfd, port_number, max_con;
	socklen_t cli_len;
	struct sockaddr_in cli_addr;
   	mutexQueue = SDL_CreateMutex();
	mutexCantClientes = SDL_CreateMutex();
	mutexClientState = SDL_CreateMutex();
	mutexProgress = SDL_CreateMutex();
	cant_con = 0;

	//Log initialize
	slog.createFile(3);

	slog.writeLine("Starting...");

	leerXML(max_con,port_number, audit);
	slog.writeLine("Port: " + to_string(port_number));
	slog.writeLine("Max connections: " + to_string(max_con));
	slog.writeLine("Audit: " + to_string(audit));

	slog.writeLine("Leyendo XML de configuracion del juego...");
	leerXMLEscenario();

	createAndDetachThread(exitManager,"exitManager", 0);
	sockfd = openAndBindSocket(port_number);

	createAndDetachThread(processMessages,"processMessages", 0);

	cli_len = sizeof(cli_addr);

	struct gst* conMsg[2];
	char* buffer;
	int playerId = 1, bufferLen, newId, tempProgress;
	string newUsername;
	progress = new unsigned int[cantJug];
	resetProgress();

	 while (1) {

		 cout << "Current connections: " << cant_con << " \n";

		 //Accept connection from client
		 newsockfd = accept(sockfd, (sockaddr *) &cli_addr, &cli_len);

		 struct timeval timeout;
		 timeout.tv_sec = 10;
		 timeout.tv_usec = 0;

		 if (setsockopt (newsockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
			 slog.writeErrorLine("ERROR setting socket rcv timeout");

		 if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		 	slog.writeErrorLine("ERROR setting socket snd timeout");

		 if (newsockfd < 0) {
			slog.writeErrorLine("ERROR on accept");
		 }else{

			 conMsg[0] = genAdminGst(playerId, command::CON_SUCCESS);
			 bufferLen = encodeMessages(&buffer, conMsg, 1);
			 send(newsockfd, buffer, bufferLen ,0);
			 delete buffer;

			 //Leo el nombre
			 buffer = new char[BUFLEN];
			 bzero(buffer,BUFLEN);
			 bufferLen = recv(newsockfd, buffer, bufferLen, 0);

			 newUsername = string(buffer);
			 bool alreadyAPlayer = (users.find(newUsername) != users.end());
			 //newId = users[newUsername];

			if (!alreadyAPlayer && (cant_con == cantJug || readyToStart) ){
				//Game is not accepting new players
				conMsg[0] = genAdminGst(0, command::CON_FAIL);
				bufferLen = encodeMessages(&buffer, conMsg, 1);
				send(newsockfd, buffer, bufferLen ,0);
				close(newsockfd);
				delete buffer;
			}else{

				if (!alreadyAPlayer){
					conMsg[0] = genAdminGst(playerId, command::CON_SUCCESS);
					Elemento* tempElem = genNewPlayer(playerId, newUsername);
					conMsg[1] = genInitGst(playerId, 1, tempElem -> getPosX(), tempElem -> getPosY(), playing);
				socket_to_id[newsockfd] = playerId;
				}
				else{
					newId = users[newUsername];
					tempProgress = getProgress();
					updateProgress(newId, tempProgress);
					conMsg[0] = genAdminGst(newId, command::CON_SUCCESS);
					conMsg[1] = genInitGst(newId, getProgress(), elementos[newId] -> getPosX(), elementos[newId] -> getPosY(), playing);
				socket_to_id[newsockfd] = playerId;
				}

				bufferLen = encodeMessages(&buffer, conMsg, 2);
				delete conMsg[0];
				delete conMsg[1];

				if (audit)
					cout << "AUDIT snd: " << buffer << endl;
				send(newsockfd, buffer, bufferLen, 0);
				if (SDL_LockMutex(mutexCantClientes) == 0) {
					cant_con++;
					readyToStart = (cant_con == cantJug);
					SDL_UnlockMutex(mutexCantClientes);
				}
				if (SDL_LockMutex(mutexClientState) == 0) {
				   	client_state[newsockfd] = true;
				   	SDL_UnlockMutex(mutexClientState);
				}
				manageNewConnection(newsockfd);
				cout << "El usuario " << newUsername << " se ha conectado corretamente." << endl;
				delete buffer;
				playerId++;

			}
		}
	}
    return 1;
}

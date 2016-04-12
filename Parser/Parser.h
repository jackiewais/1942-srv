/*
 * Parser.h
 *
 *  Created on: Mar 29, 2016
 *      Author: bigfatpancha
 */


#include <map>
#include "../Logger/Log.h"

namespace Parser {

	enum messageType {CHAR, INT, DOUBLE, STRING, ERROR};

	struct type_datosServer {
		int cantMaxClientes;
		int puerto;
		unsigned short logLevel;
	};

	struct type_mensaje {
		char id[10];
		messageType tipo;
		char* valor;
	};

	struct type_datosCliente {
		char * ip;
		int puerto;
		unsigned short logLevel;
		std::map<int,type_mensaje> * mensajes;
	};

	bool fileExists(const char* name);

	const char* getDefaultNameServer();
	const char* getDefaultNameClient();

	type_datosServer parseXMLServer(const char * nombreArchivo, Log * log);
	type_datosCliente parseXMLCliente(const char * nombreArchivo, Log * log);

};



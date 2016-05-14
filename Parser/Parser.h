/*
 * Parser.h
 *
 *  Created on: Mar 29, 2016
 *      Author: bigfatpancha
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <map>
#include <iostream>
#include <string>
#include <list>
#include "../Logger/Log.h"

using namespace std;

namespace Parser {

	// ================================================================================
	// DATOS RELACIONADOS CON LA PRIMER ENTREGA.
	// ================================================================================

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

	// ================================================================================


	// ================================================================================
	// DATOS RELACIONADOS CON LA SEGUNDA ENTREGA.
	// ================================================================================

	enum spriteType {DISPARO, VUELTA, PELOTA, ENFERMERA, JUGADOR, FONDO, BLANCO};

	struct type_Ventana{
		int ancho;
		int alto;
	};

	struct type_Sprite{
		spriteType id;
		char* path;
		int cantidad;
		int ancho;
		int alto;
	};

	struct type_Fondo{
		spriteType spriteId;
		int ancho;
		int alto;
	};

	struct type_Elemento{
		spriteType spriteId;
		int posicionX;
		int posicionY;
	};

	struct type_Escenario{
		int ancho;
		int alto;
		type_Fondo fondo;
		list<type_Elemento> elementos;
	};

	struct type_Avion{
		int velocidadDesplazamiento;
		int velocidadDisparos;
		spriteType avionSpriteId;
		spriteType vueltaSpriteId;
		spriteType disparosSpriteId;
	};

	struct type_DatosGraficos{
		int logLevel;
		type_Ventana ventana;
		list<type_Sprite> sprites;
		type_Escenario escenario;
		type_Avion avion;
	};

	// ================================================================================

	bool fileExists(const char* name);

	const char* getDefaultNameServer();
	const char* getDefaultNameClient();
	const char* getDefaultNameServerMap();

	// ================================================================================
	// METODOS RELACIONADOS CON LA PRIMER ENTREGA.
	// ================================================================================

	type_datosServer parseXMLServer(const char * nombreArchivo, Log * log);
	type_datosCliente parseXMLCliente(const char * nombreArchivo, Log * log);

	// ================================================================================

	// ================================================================================
	// METODOS RELACIONADOS CON LA SEGUNDA ENTREGA.
	// ================================================================================

	type_DatosGraficos parseXMLServerMap(const char * nombreArchivo, Log * log);

	// ================================================================================
};

#endif /* PARSER_H_ */

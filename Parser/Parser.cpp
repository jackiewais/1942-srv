/*
 * Parser.cpp

 *
 *  Created on: Mar 29, 2016
 *  Author: bigfatpancha
 */
#include "Parser.h"
#include "DefaultErrorHandler.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <map>
#include "../Logger/Log.h"
#include "rapidxml-1.13/rapidxml.hpp"
#include "xercesc/util/PlatformUtils.hpp"
#include "xercesc/parsers/XercesDOMParser.hpp"
#include <arpa/inet.h>

using namespace std;
using namespace rapidxml;
using namespace Parser;
using namespace xercesc;

char* toUpercase(char* tipo){
	int i = 0;
	char c;
	while (tipo[i]){
		c = tipo[i];
		tipo[i] = toupper(c);
		i++;
	}
	return tipo;
}

messageType formatTipoMsj(char * tipo) {
	if (strcmp(tipo,"INT")==0 || strcmp(tipo,"INTEGER")==0)
		return INT;
	if (strcmp(tipo,"STRING")==0 || strcmp(tipo,"STR")==0)
		return STRING;
	if (strcmp(tipo,"CHAR")==0)
		return CHAR;
	if (strcmp(tipo,"DOUBLE")==0)
		return DOUBLE;
	return ERROR;
}

bool sonDigitos(char* str){
	for (unsigned int i = 0; i < strlen (str); i++) {
		if (! isdigit (str[i])) {
			return false;
		}
	}
	return true;
}

bool validarLogLevel(char* level){
	return sonDigitos(level);
}

bool validarCamposServer(char* cantMax, char* puerto){
	if (sonDigitos(cantMax) && sonDigitos(puerto))
		return true;
	return false;
}

bool validarIpYPuerto(char * ip, char *puerto){
	unsigned char buf[sizeof(struct in6_addr)];
	int ipValida = inet_pton(AF_INET, ip, buf);
	if (ipValida == 0){
		return false;
	}
	if (!sonDigitos(puerto)) {
		return false;
	}
	return true;
}

bool validarMensaje(char * id, messageType tipo){
	if (strlen(id) > 10){
		return false;
	}
	if (tipo == (messageType) ERROR){
		return false;
	}
	return true;
}

bool Parser::fileExists(const char* name) {
	return ( access( name, F_OK ) != -1 );
}

const char* Parser::getDefaultNameServer(){
	return "defaultServerXML.xml";
}

const char* Parser::getDefaultNameServerMap(){
	return "defaultServerMapXML.xml";
}

const char* Parser::getDefaultNameClient(){
	return "defaultClienteXML.xml";
}


bool validarUnicidadID(map<int,type_mensaje> * mensajes) {
	int totMsj = mensajes->size();
	type_mensaje mensaje;
	char id[10] = {};
	type_mensaje mensajeAct;
	char idAct[10] = {};
	for (int i = 0; i < totMsj; i++){
		mensaje = mensajes->find(i)->second;
		strcpy(id, mensaje.id);
		for (int j = 0; j < totMsj; j++){
			mensajeAct = mensajes->find(j)->second;
			strcpy(idAct, mensajeAct.id);
			if ((i != j) && (strcmp(id, idAct) == 0)){
				return false;
			}
		}
	}
	return true;
}

void validarFormato(const char * nombreArchivo,const char * schemaPath, bool &XMLValido){
	XMLPlatformUtils::Initialize();
	{
		XercesDOMParser domParser;
		if (domParser.loadGrammar(schemaPath, Grammar::SchemaGrammarType) == NULL){
			XMLValido = false;

			cout << "EN: loadGrammar" << endl;
		} else {
			DefaultErrorHandler * errorHandler = new DefaultErrorHandler;

			domParser.setErrorHandler(errorHandler);
			domParser.setValidationScheme(XercesDOMParser::Val_Always);
			domParser.setDoNamespaces(true);
			domParser.setDoSchema(true);
			domParser.setValidationSchemaFullChecking(true);

			domParser.setExternalNoNamespaceSchemaLocation(schemaPath);

			domParser.parse(nombreArchivo);
			if(domParser.getErrorCount() != 0){
				XMLValido = false;
				cout << "EN: getErrorCount" << endl;
			}
			else
				XMLValido = true;
		}
	}
	XMLPlatformUtils::Terminate();
}

type_datosServer Parser::parseXMLServer(const char * nombreArchivo, Log * log) {
	xml_document<> archivo;
	xml_node<> * nodo_raiz;

	ifstream elArchivo (nombreArchivo);
	if (!elArchivo.good()) {
		log->writeErrorLine("Archivo inexistente");
		return parseXMLServer(getDefaultNameServer(), log);
	}

	bool formatoCorrecto;
	validarFormato(nombreArchivo, "schemaServer.xsd", formatoCorrecto);
	if (!formatoCorrecto){
		log->writeErrorLine("Archivo con formato invalido");
		return parseXMLServer(getDefaultNameServer(), log);
	}

	vector<char> buffer((istreambuf_iterator<char>(elArchivo)), istreambuf_iterator<char>());
	buffer.push_back('\0');

	archivo.parse<0>(&buffer[0]);

	nodo_raiz = archivo.first_node("servidor");
	xml_node<> * nodo_cantMaxCli = nodo_raiz->first_node("CantidadMaximaClientes");
	char* cantMax = nodo_cantMaxCli->value();
	xml_node<> * nodo_puerto = nodo_raiz->first_node("puerto");
	char* puerto = nodo_puerto->value();
	xml_node<> * nodo_log = nodo_raiz->first_node("log");
	char* level = nodo_log->value();
	xml_node<> * nodo_audit = nodo_raiz->first_node("audit");
	char* audit = nodo_audit->value();

	if (validarCamposServer(cantMax, puerto)) {
		type_datosServer unXML;
		unXML.cantMaxClientes = atoi(cantMax);
		unXML.puerto = atoi(puerto);
		unXML.logLevel = atoi(level);
		unXML.audit = atoi(audit);
		return unXML;
	} else {
		log->writeErrorLine("puerto y/o cantidad maxima invalidos");
		return parseXMLServer(getDefaultNameServer(), log);
	}

}

type_datosCliente Parser::parseXMLCliente(const char * nombreArchivo, Log* log) {
	xml_document<> archivo;
	xml_node<> * nodo_raiz;

	ifstream elArchivo (nombreArchivo);
	if (!elArchivo.good()) {
		log->writeErrorLine("Archivo inexistente, se usa el default");
		return parseXMLCliente(getDefaultNameClient(), log);
	}

	bool formatoCorrecto;
	validarFormato(nombreArchivo, "schemaCliente.xsd", formatoCorrecto);
	if (!formatoCorrecto){
		log->writeErrorLine("Archivo con formato invalido, se usa el default");
		return parseXMLCliente(getDefaultNameClient(), log);
	}

	vector<char> buffer((istreambuf_iterator<char>(elArchivo)), istreambuf_iterator<char>());
	buffer.push_back('\0');

	archivo.parse<0>(&buffer[0]);

	nodo_raiz = archivo.first_node("Cliente");

	type_datosCliente unXML;

	xml_node<> * nodo_conexion = nodo_raiz->first_node("conexion");
	xml_node<> * nodo_log = nodo_raiz->first_node("log");
	char* level = nodo_log->value();
	xml_node<> * nodo_ip = nodo_conexion->first_node("IP");
	char* ip = nodo_ip->value();
	xml_node<> * nodo_puerto = nodo_conexion->first_node("puerto");
	char* puerto = nodo_puerto->value();

	if (validarIpYPuerto(ip, puerto) && validarLogLevel(level)) {
		unXML.ip = ip;
		unXML.puerto = atoi(puerto);
		unXML.logLevel = atoi(level);
	} else {
		log->writeErrorLine("ip y/o puerto invalidos");
		return parseXMLCliente(getDefaultNameClient(), log);
	}

	map<int,type_mensaje>* mensajes = new map<int,type_mensaje>();
		int i = 0;
	messageType tipo;
	char id[10] = {};
	char * valor;
	for (xml_node<> * nodo_msj = nodo_raiz->first_node("mensaje"); nodo_msj; nodo_msj = nodo_msj->next_sibling()) {
		type_mensaje unMsj;
		xml_node<> * nodo_id = nodo_msj->first_node("id");
		strcpy(id, nodo_id->value());
		xml_node<> * nodo_tipo = nodo_msj->first_node("tipo");
		tipo = formatTipoMsj(nodo_tipo->value());
		xml_node<> * nodo_valor = nodo_msj->first_node("valor");
		valor = nodo_valor->value();

		if (validarMensaje(id, tipo)) {
			strcpy(unMsj.id, id);
			unMsj.tipo = tipo;
			unMsj.valor = valor;
			(*mensajes).insert(std::pair<int,type_mensaje>(i,unMsj));
			i++;
		} else {
			log->writeErrorLine("mensaje cliente invalido");
			return parseXMLCliente(getDefaultNameClient(), log);
		}
	}
	if (!validarUnicidadID(mensajes)) {
		log->writeErrorLine("ids duplicados");
		return parseXMLCliente(getDefaultNameClient(), log);
	}
	unXML.mensajes = mensajes;
	return unXML;
}


class myexception: public exception
{
  virtual const char* what() const throw() { return "METODO EN DESARROLLO!"; }
} NotImplementedException;

// Los tipos de Sprites son los siguientes: DISPARO, VUELTA, PELOTA, ENFERMERA, JUGADOR, BLANCO, FONDO
spriteType formatTipoSprite(char * tipo) {
	if (strcmp(tipo,"PELOTA")==0)
		return PELOTA;
	if (strcmp(tipo,"DISPARO")==0)
		return DISPARO;
	if (strcmp(tipo,"VUELTA")==0)
		return VUELTA;
	if (strcmp(tipo, "JUGADOR")==0)
		return JUGADOR;
	if (strcmp(tipo, "JUGADOR")==0)
		return JUGADOR;
	if (strcmp(tipo, "ENFERMERA")==0)
		return ENFERMERA;
	if (strcmp(tipo, "FONDO")==0)
		return FONDO;
	return BLANCO;
}

//===================VALORES DEFAULT====================================
string default_level="1";
string default_ventana_ancho = "800";
string default_ventana_alto = "600";
string default_sprite_ancho = "10";
string default_sprite_alto = "10";
string default_sprite_cantidad = "1";
string default_escenario_alto = "1000";
string default_escenario_ancho = "100";
string default_fondo_alto = "10";
string default_fondo_ancho = "10";
string default_elemento_posicion_x = "10";
string default_elemento_posicion_y = "15";
string default_velDesplazamiento = "10";
string default_velocidadDisparos = "6";
string default_sprite_path = "/home/lucia/git/game-1986/vacio.bmp";
//===================VALORES DEFAULT====================================


type_DatosGraficos Parser::parseXMLServerMap(const char * nombreArchivo, Log * log){
	xml_document<> archivo;
	xml_node<> * nodo_raiz;

	log->writeLine("Comenzamos a validar la EXISTENCIA del archivo.");
	// Chequeamos la existencia del archivo ingresado por el cliente.
	ifstream elArchivo (nombreArchivo);
	if (!elArchivo.good()) {
		log->writeErrorLine("Archivo inexistente, se usa el default");
		return parseXMLServerMap(getDefaultNameServerMap(), log);
	}
	log->writeLine("Validación exitosa sobre la EXISTENCIA del archivo.");

	log->writeLine("Comenzamos a validar la ESTRUCTURA del archivo.");
	// Validamos el formato y la estructura del archivo.
	bool formatoCorrecto;
	validarFormato(nombreArchivo, "schemaServerMap.xsd", formatoCorrecto);
	if (!formatoCorrecto){
		log->writeErrorLine("Archivo con formato invalido, se usa el default.");
		return parseXMLServerMap(getDefaultNameServerMap(), log);
	}
	log->writeLine("Validación exitosa sobre la ESTRUCTURA del archivo.");

	vector<char> buffer((istreambuf_iterator<char>(elArchivo)), istreambuf_iterator<char>());
	buffer.push_back('\0');

	archivo.parse<0>(&buffer[0]);

	nodo_raiz = archivo.first_node("servidor");

	type_DatosGraficos graficos;
	type_Avion avion;
	type_Escenario escenario;
	type_Fondo fondo;
	type_Ventana ventana;

	log->writeLine("Comenzamos a validar y obtener los datos de la cantidad de jugadores.");
	xml_node<> * nodo_cantJug = nodo_raiz->first_node("cantidadDeJugadores");
	char * cantJug = nodo_cantJug->value();
	graficos.cantJug = atoi(cantJug);
	log->writeLine("Hemos obtenido los datos de la cantidad de jugadores exitosamente.");

	log->writeLine("Comenzamos a validar y obtener los datos sobre el LOG del archivo.");
	//===========================LOG===============================
	// Obtenemos la información del nodo referente al LOG.
	xml_node<> * nodo_log = nodo_raiz->first_node("log");
	char* level = nodo_log->value();
	// Validamos la información contenida en el nodo: LOG.
	if(!sonDigitos(level)){ level = strdup(default_level.c_str());
		log->writeWarningLine("Valor para el LOG invalido, usamos el valor por defecto.");}
	// Resguardamos la información obtenida en el nodo: LOG.
	graficos.logLevel = atoi(level);
	//===========================LOG===============================
	log->writeLine("Hemos obtenido los datos sobre el LOG del archivo exitosamente.");

	log->writeLine("Comenzamos a validar y obtener los datos sobre la VENTANA del archivo.");
	//=========================VENTANA=============================
	// Obtenemos la información del nodo referente a VENTANA.
	xml_node<> * nodo_ventana = nodo_raiz->first_node("ventana");
	xml_node<> * nodo_ventana_ancho = nodo_ventana->first_node("ancho");
	char* ventana_ancho = nodo_ventana_ancho->value();
	xml_node<> * nodo_ventana_alto = nodo_ventana->first_node("alto");
	char* ventana_alto = nodo_ventana_alto->value();
	// Validamos la información contenida en el nodo: VENTANA.
	if(!sonDigitos(ventana_ancho) || !sonDigitos(ventana_alto))
	{ 	ventana_ancho=strdup(default_ventana_ancho.c_str());
		ventana_alto=strdup(default_ventana_alto.c_str());
		log->writeWarningLine("Valoar para el ancho y alto de la VENTANA invalido, usamos valores por defecto.");}
	// Resguardamos la información obtenida en el nodo: VENTANA.
	ventana.ancho = atoi(ventana_ancho);
	ventana.alto = atoi(ventana_alto);
	graficos.ventana = ventana;
	//=========================VENTANA=============================
	log->writeLine("Hemos obtenido los datos sobre la VENTANA del archivo exitosamente.");

	log->writeLine("Comenzamos a validar y obtener los datos sobre los SPRITES del archivo.");
	//==========================SPRITE=============================
	// Obtenemos la información del nodo referente a SPRITES.
	xml_node<> * nodo_sprites = nodo_raiz->first_node("sprites");
	char* sprite_id; char* sprite_path; char* sprite_cantidad;
	char* sprite_ancho; char* sprite_alto;
	// Recorremos cada sprite dentro de la nodo que los incluye.
	for (xml_node<> * nodo_sprite = nodo_sprites->first_node("sprite"); nodo_sprite; nodo_sprite = nodo_sprite->next_sibling()) {

		type_Sprite sprite;

		xml_node<> * nodo_sprite_id = nodo_sprite->first_node("id");
		sprite_id = nodo_sprite_id->value();
		xml_node<> * nodo_sprite_path = nodo_sprite->first_node("path");
		sprite_path = nodo_sprite_path->value();
		xml_node<> * nodo_sprite_cantidad = nodo_sprite->first_node("cantidad");
		sprite_cantidad = nodo_sprite_cantidad->value();
		xml_node<> * nodo_sprite_ancho = nodo_sprite->first_node("ancho");
		sprite_ancho = nodo_sprite_ancho->value();
		xml_node<> * nodo_sprite_alto = nodo_sprite->first_node("alto");
		sprite_alto = nodo_sprite_alto->value();
		// Validamos la información contenida en el nodo: SPRITES.
		if(!sonDigitos(sprite_ancho) || !sonDigitos(sprite_alto))
		{ 	sprite_ancho = strdup(default_sprite_ancho.c_str());
			sprite_alto = strdup(default_sprite_alto.c_str());
			log->writeWarningLine("Valoar para el ancho y alto del SPRITE invalido, usamos valores por defecto.");}
		if(!sonDigitos(sprite_cantidad))
		{ sprite_cantidad =  strdup(default_sprite_cantidad.c_str());
		log->writeWarningLine("Valoar para la cantidad del SPRITE invalido, usamos valores por defecto.");}
		// Chequeamos la existencia del archivo imagen en cuestión.
		ifstream elSprite (sprite_path);
		if (!elSprite.good()) {
			sprite_path = strdup(default_sprite_path.c_str());
			log->writeWarningLine("Archivo inexistente para el path del SPRITE, se usa la imagen default.");
		}
		// Resguardamos la información obtenida en el nodo: SPRITES.
		sprite.id = formatTipoSprite(toUpercase(sprite_id));
		sprite.path = sprite_path;
		sprite.cantidad;
		sprite.ancho;
		sprite.alto;
		// Lo agregamos a la lista principal de SPRITES.
		graficos.sprites.push_back(sprite);
	}
	//==========================SPRITE=============================
	log->writeLine("Hemos obtenido los datos sobre los SPRITES del archivo exitosamente.");

	log->writeLine("Comenzamos a validar y obtener los datos sobre el ESCENARIO del archivo.");
	//========================ESCENARIO============================
	// Obtenemos la información del nodo referente a ESCENARIO.
	xml_node<> * nodo_escenario = nodo_raiz->first_node("escenario");
	xml_node<> * nodo_escenario_ancho = nodo_escenario->first_node("ancho");
	char* escenario_ancho = nodo_escenario_ancho->value();
	xml_node<> * nodo_escenario_alto = nodo_escenario->first_node("alto");
	char* escenario_alto = nodo_escenario_alto->value();
	// Validamos la información contenida en el nodo: ESCENARIO.
	if(!sonDigitos(escenario_ancho) || !sonDigitos(escenario_alto))
	{ 	escenario_ancho = strdup(default_escenario_ancho.c_str());
		escenario_alto = strdup(default_escenario_alto.c_str());
		log->writeWarningLine("Valoar para el ancho y alto del ESCENARIO invalido, usamos valores por defecto.");}
	// Resguardamos la información obtenida en el nodo: ESCENARIO.
	escenario.ancho = atoi(escenario_ancho);
	escenario.alto = atoi(escenario_alto);
	//========================ESCENARIO============================
	log->writeLine("Hemos obtenido los datos sobre el ESCENARIO del archivo exitosamente.");

	log->writeLine("Comenzamos a validar y obtener los datos sobre el FONDO del archivo.");
	//=========================FONDO==============================
	// ESCENARIO >> Obtenemos la información del nodo referente a FONDO.
	xml_node<> * nodo_fondo = nodo_escenario->first_node("fondo");
	xml_node<> * nodo_spriteId = nodo_fondo->first_node("spriteId");
	char* fondo_sprite_Id = nodo_spriteId->value();
	xml_node<> * nodo_fondo_ancho = nodo_fondo->first_node("ancho");
	char* fondo_ancho = nodo_fondo_ancho->value();
	xml_node<> * nodo_fondo_alto = nodo_fondo->first_node("alto");
	char* fondo_alto = nodo_fondo_alto->value();
	// Validamos la información contenida en el nodo: FONDO.
	if(!sonDigitos(fondo_ancho) || !sonDigitos(fondo_alto))
	{ 	fondo_ancho = strdup(default_fondo_ancho.c_str());
		fondo_alto = strdup(default_fondo_alto.c_str());
		log->writeWarningLine("Valoar para el ancho y alto del FONDO invalido, usamos valores por defecto.");}
	// Resguardamos la información obtenida en el nodo: FONDO.
	fondo.ancho = atoi(fondo_ancho);
	fondo.alto = atoi(fondo_alto);
	fondo.spriteId = formatTipoSprite(toUpercase(fondo_sprite_Id));
	escenario.fondo = fondo;
	//=========================FONDO==============================
	log->writeLine("Hemos obtenido los datos sobre el FONDO del archivo exitosamente.");

	log->writeLine("Comenzamos a validar y obtener los datos sobre los ELEMENTOS del archivo.");
	//=======================ELEMENTO=============================
	// ESCENARIO >> Obtenemos la información del nodo referente a ELEMENTOS.
	xml_node<> * nodo_elementos = nodo_escenario->first_node("elementos");
	char* sprite_elemento_id;
	char* elemento_posicion_x;
	char* elemento_posicion_y;
	// Recorremos cada sprite dentro de la nodo que los incluye.
	for (xml_node<> * nodo_elemento = nodo_elementos->first_node("elemento"); nodo_elemento; nodo_elemento = nodo_elemento->next_sibling()) {

		type_Elemento elemento;

		xml_node<> * nodo_elemento_spriteId = nodo_elemento->first_node("spriteId");
		sprite_elemento_id = nodo_elemento_spriteId->value();
		xml_node<> * nodo_elemento_posicion = nodo_elemento->first_node("posicion");
		xml_node<> * nodo_elemento_posicion_x = nodo_elemento_posicion->first_node("x");
		elemento_posicion_x = nodo_elemento_posicion_x->value();
		xml_node<> * nodo_elemento_posicion_y = nodo_elemento_posicion->first_node("y");
		elemento_posicion_y = nodo_elemento_posicion_y->value();
		// Validamos la información contenida en el nodo: ELEMENTOS.
		if(!sonDigitos(elemento_posicion_x) || !sonDigitos(elemento_posicion_y))
		{ 	elemento_posicion_x = strdup(default_elemento_posicion_x.c_str());
			elemento_posicion_y = strdup(default_elemento_posicion_y.c_str());
			log->writeWarningLine("Valoar para la posición del ELEMENTO invalido, usamos valores por defecto.");}
		// Resguardamos la información obtenida en el nodo: ELEMENTOS.
		elemento.spriteId = formatTipoSprite(toUpercase(sprite_elemento_id));
		elemento.posicionX = atoi(elemento_posicion_x);
		elemento.posicionY = atoi(elemento_posicion_y);
		escenario.elementos.push_back(elemento);
	}
	graficos.escenario = escenario;
	//=======================ELEMENTO=============================
	log->writeLine("Hemos obtenido los datos sobre los ELEMENTOS del archivo exitosamente.");

	log->writeLine("Comenzamos a validar y obtener los datos sobre el AVION del archivo.");
	//==========================AVION==============================
	// Obtenemos la información del nodo referente a AVION.
	xml_node<> * nodo_avion = nodo_raiz->first_node("avion");
	xml_node<> * nodo_velDesplazamiento = nodo_avion->first_node("velDesplazamiento");
	char* velDesplazamiento = nodo_velDesplazamiento->value();
	xml_node<> * nodo_velocidadDisparos = nodo_avion->first_node("velocidadDisparos");
	char* velocidadDisparos = nodo_velocidadDisparos->value();
	xml_node<> * nodo_avionSpriteId = nodo_avion->first_node("avionSpriteId");
	char* avionSpriteId = nodo_avionSpriteId->value();
	xml_node<> * nodo_vueltaSpriteId = nodo_avion->first_node("vueltaSpriteId");
	char* vueltaSpriteId = nodo_vueltaSpriteId->value();
	xml_node<> * nodo_disparosSpriteId = nodo_avion->first_node("disparosSpriteId");
	char* disparosSpriteId = nodo_disparosSpriteId->value();
	// Validamos la información contenida en el nodo: AVION
	if(!sonDigitos(velDesplazamiento))
	{ velDesplazamiento =  strdup(default_velDesplazamiento.c_str());
	log->writeWarningLine("Valoar para la VELOCIDAD DE DESPLAZAMIENTO invalido, usamos valores por defecto.");}
	if(!sonDigitos(velocidadDisparos))
	{ velocidadDisparos =  strdup(default_velocidadDisparos.c_str());
	log->writeWarningLine("Valoar para la VELOCIDAD DE DISPAROS invalido, usamos valores por defecto.");}
	// Resguardamos la información obtenida en el nodo: AVION.
	avion.velocidadDesplazamiento = atoi(velDesplazamiento);
	avion.velocidadDisparos = atoi(velocidadDisparos);
	avion.avionSpriteId = formatTipoSprite(toUpercase(avionSpriteId));
	avion.vueltaSpriteId = formatTipoSprite(toUpercase(vueltaSpriteId));
	avion.disparosSpriteId = formatTipoSprite(toUpercase(disparosSpriteId));
	graficos.avion = avion;
	//==========================AVION==============================
	log->writeLine("Hemos obtenido los datos sobre el AVION del archivo exitosamente.");

	log->writeLine("Devolvemos los datos del XML parseados.");
	// Terminamos devolviendo el elemento del tipo: type_DatosGraficos.
	return graficos;
}


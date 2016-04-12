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
	for (int i = 0; i < strlen (str); i++) {
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

void validarFormato(const char * nombreArchivo, char * schemaPath, bool &XMLValido){
	XMLPlatformUtils::Initialize();
	{
		XercesDOMParser domParser;
		if (domParser.loadGrammar(schemaPath, Grammar::SchemaGrammarType) == NULL){
			XMLValido = false;
		} else {
			DefaultErrorHandler * errorHandler = new DefaultErrorHandler;

			domParser.setErrorHandler(errorHandler);
			domParser.setValidationScheme(XercesDOMParser::Val_Always);
			domParser.setDoNamespaces(true);
			domParser.setDoSchema(true);
			domParser.setValidationSchemaFullChecking(true);

			domParser.setExternalNoNamespaceSchemaLocation(schemaPath);

			domParser.parse(nombreArchivo);
			if(domParser.getErrorCount() != 0)
				XMLValido = false;
			else
				XMLValido = true;
		}
	}
	XMLPlatformUtils::Terminate();
}

type_datosServer Parser::parseXMLServer(const char * nombreArchivo) {
	xml_document<> archivo;
	xml_node<> * nodo_raiz;

	ifstream elArchivo (nombreArchivo);
	if (!elArchivo.good()) {
		return parseXMLServer(getDefaultNameServer());
	}

	bool formatoCorrecto;
	validarFormato(nombreArchivo, "schemaServer.xsd", formatoCorrecto);
	if (!formatoCorrecto)
		return parseXMLServer(getDefaultNameServer());

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

	if (validarCamposServer(cantMax, puerto)) {
		type_datosServer unXML;
		unXML.cantMaxClientes = atoi(cantMax);
		unXML.puerto = atoi(puerto);
		unXML.logLevel = atoi(level);
		return unXML;
	} else
		return parseXMLServer(getDefaultNameServer());

}

const char* Parser::getDefaultNameClient(){
	return "defaultClienteXML.xml";
}

type_datosCliente Parser::parseXMLCliente(const char * nombreArchivo) {
	xml_document<> archivo;
	xml_node<> * nodo_raiz;

	ifstream elArchivo (nombreArchivo);
	if (!elArchivo.good()) {
		return parseXMLCliente(getDefaultNameClient());
	}

	bool formatoCorrecto;
	validarFormato(nombreArchivo, "schemaCliente.xsd", formatoCorrecto);
	if (!formatoCorrecto)
		return parseXMLCliente(getDefaultNameClient());

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
	} else
		return parseXMLCliente(getDefaultNameClient());

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
		} else
			return parseXMLCliente(getDefaultNameClient());
	}
	if (!validarUnicidadID(mensajes))
		return parseXMLCliente(getDefaultNameClient());
	unXML.mensajes = mensajes;
	return unXML;
}



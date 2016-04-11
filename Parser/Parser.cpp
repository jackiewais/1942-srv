/*
 * Parser.cpp

 *
 *  Created on: Mar 29, 2016
 *  Author: bigfatpancha
 */
#include "Parser.h"
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
#include "rapidxml-1.13/rapidxml.hpp"

using namespace std;
using namespace rapidxml;
using namespace Parser;

messageType formatTipoMsj(char * tipo) {
	if (strcmp(tipo,"INT")==0 || strcmp(tipo,"int")==0 || strcmp(tipo,"Integer")==0)
		return INT;
	if (strcmp(tipo,"String")==0 || strcmp(tipo,"str")==0 || strcmp(tipo,"STR")==0)
		return STRING;
	if (strcmp(tipo,"char")==0 || strcmp(tipo,"CHAR")==0)
		return CHAR;
	if (strcmp(tipo,"double")==0 || strcmp(tipo,"Double")==0 || strcmp(tipo,"DOUBLE")==0)
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

bool validarCamposServer(char* cantMax, char* puerto){
	if (sonDigitos(cantMax) && sonDigitos(puerto))
		return true;
	return false;
}

bool validarIpYPuerto(char * ip, char *puerto){
	if (sonDigitos(puerto))
		return true;
	return false;
	//TODO falta validar IP
}

bool validarMensaje(char * id, messageType tipo, char * valor){
	if (!sonDigitos(id))
		return false;
	if (tipo == (messageType) ERROR)
		return false;
	//char * val = (char*) &valor;
	if ((tipo == (messageType) INT || tipo == (messageType) DOUBLE) && !sonDigitos(valor))
		return false;
	if ((tipo == (messageType) CHAR) && strlen(valor) > 1)
		return false;
	if ((tipo == (messageType) STRING) && strlen(valor) <= 1)
		return false;
	return true;
}

bool Parser::fileExists(const char* name) {
	return ( access( name, F_OK ) != -1 );
}

const char* Parser::getDefaultNameServer(){
	return "Parser/defaultServerXML.xml";
}

type_datosServer Parser::parseXMLServer(const char* nombreArchivo) {
	xml_document<> archivo;
	xml_node<> * nodo_raiz;

	ifstream elArchivo (nombreArchivo);
	if (elArchivo.bad()) {
		return parseXMLServer(getDefaultNameServer());
	}

	vector<char> buffer((istreambuf_iterator<char>(elArchivo)), istreambuf_iterator<char>());
	buffer.push_back('\0');

	archivo.parse<0>(&buffer[0]);

	nodo_raiz = archivo.first_node("servidor");
	xml_node<> * nodo_cantMaxCli = nodo_raiz->first_node("CantidadMaximaClientes");
	char* cantMax = nodo_cantMaxCli->value();
	xml_node<> * nodo_puerto = nodo_raiz->first_node("puerto");
	char* puerto = nodo_puerto->value();

	if (validarCamposServer(cantMax, puerto)) {
		type_datosServer unXML;
		unXML.cantMaxClientes = atoi(cantMax);
		unXML.puerto = atoi(puerto);
		return unXML;
	} else
		return parseXMLServer(getDefaultNameServer());

}


const char* Parser::getDefaultNameClient(){
	return "Parser/defaultClienteXML.xml";
}
type_datosCliente Parser::parseXMLCliente(const char* nombreArchivo) {
	xml_document<> archivo;
	xml_node<> * nodo_raiz;

	ifstream elArchivo (nombreArchivo);
	if (!elArchivo.good()) {
		return parseXMLCliente(getDefaultNameClient());
	}

	vector<char> buffer((istreambuf_iterator<char>(elArchivo)), istreambuf_iterator<char>());
	buffer.push_back('\0');

	archivo.parse<0>(&buffer[0]);

	nodo_raiz = archivo.first_node("Cliente");

	type_datosCliente unXML;

	xml_node<> * nodo_conexion = nodo_raiz->first_node("conexion");
	xml_node<> * nodo_ip = nodo_conexion->first_node("IP");
	char* ip = nodo_ip->value();
	xml_node<> * nodo_puerto = nodo_conexion->first_node("puerto");
	char* puerto = nodo_puerto->value();

	if (validarIpYPuerto(ip, puerto)) {
		unXML.ip = ip;
		unXML.puerto = atoi(puerto);
	} else
		return parseXMLCliente(getDefaultNameClient());

	map<int,type_mensaje>* mensajes = new map<int,type_mensaje>();
	int i = 0;
	messageType tipo;
	char * id;
	char * valor;
	for (xml_node<> * nodo_msj = nodo_raiz->first_node("mensaje"); nodo_msj; nodo_msj = nodo_msj->next_sibling()) {
		type_mensaje unMsj;
		xml_node<> * nodo_id = nodo_msj->first_node("id");
		id = nodo_id->value();
		xml_node<> * nodo_tipo = nodo_msj->first_node("tipo");
		tipo = formatTipoMsj(nodo_tipo->value());
		xml_node<> * nodo_valor = nodo_msj->first_node("valor");
		valor = nodo_valor->value();

		if (validarMensaje(id, tipo, valor)) {
			unMsj.id = atoi(id);
			unMsj.tipo = tipo;
			unMsj.valor = valor;
			(*mensajes).insert(std::pair<int,type_mensaje>(i,unMsj));
			i++;
		} else
			return parseXMLCliente("defaultClienteXML.xml");
	}
	unXML.mensajes = mensajes;

	return unXML;
}



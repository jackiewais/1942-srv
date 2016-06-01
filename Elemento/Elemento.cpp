/*
 * Elemento.cpp
 *
 *  Created on: 2 de may. de 2016
 *      Author: pablomd
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "Elemento.h"
#include "../Utils/messages.h"

Elemento::Elemento(int idEl, int x, int y, int cantJug) {
	id = idEl;
	posX = x;
	posY= y;
	estadoLen = cantJug;
	estado = new status[cantJug];
	for (int i = 0; i < estadoLen; i++){
		estado[i] = status::VIVO;
	}

}

Elemento::Elemento(struct gst* msg, int cantJug){

	id = atoi(msg -> id);
	posX = atoi(msg -> posx);
	posY = atoi(msg -> posy);
	estadoLen = cantJug;
	estado = new status[cantJug];
	for (int i = 0; i < estadoLen; i++){
		estado[i] = status::VIVO;
	}
}

void Elemento::update(int x, int y, status nuevoEstado){
	posX = x;
	posY = y;
	this->updateStatus(nuevoEstado);
}

void Elemento::update(struct gst* msg){
	status nuevoEstado;
	if (id == (atoi(msg -> id))){
		posX = atoi(msg -> posx);
		posY = atoi(msg -> posy);
		nuevoEstado = (status) msg -> info[0];
		this->updateStatus(nuevoEstado);
	}
}

void Elemento::updateStatus(status nuevoEstado){
	if (((nuevoEstado == status::START) ||
		(nuevoEstado == status::RESET) ||
		(nuevoEstado == status::PAUSA) ||
		(nuevoEstado == status::VIVO)) &&
		(estado[0] != status::DESCONECTADO)){
			estado[id - 1] = nuevoEstado;
	}
	else{
		for (int i = 0; i < estadoLen; i++){
			estado[i] = nuevoEstado;
		}
	}
}

void Elemento::updatePos(int x, int y){
	posX = x;
	posY = y;
}


int Elemento::getId(){
	return id;
}

int Elemento::getPosX(){
	return posX;
}

int Elemento::getPosY(){
	return posY;
}

status Elemento::getEstado(int idc){
	status temp = estado[idc -1];
	if ((temp == status::DISPARANDO) ||(temp == status::TRUCO)){
		estado[idc -1] = status::VIVO;
	}
	return temp;
}




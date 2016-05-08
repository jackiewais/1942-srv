/*
 * Elemento.cpp
 *
 *  Created on: 2 de may. de 2016
 *      Author: pablomd
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Elemento.h"
#include "../Utils/messages.h"

Elemento::Elemento(int idEl, int x, int y) {
	id = idEl;
	posX = x;
	posY= y;
	estado = VIVO;

}

Elemento::Elemento(struct gst* msg){

	id = atoi(msg -> id);
	posX = atoi(msg -> posx);
	posY = atoi(msg -> posy);
	estado = VIVO;
}

void Elemento::update(int x, int y, status nuevoEstado){
	posX = x;
	posY = y;
	estado = nuevoEstado;
}

void Elemento::update(struct gst* msg){
	if (id == (atoi(msg -> id))){
		posX = atoi(msg -> posx);
		posY = atoi(msg -> posy);
		estado = (status) msg -> info[0];
	}
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

status Elemento::getEstado(){
	return estado;
}




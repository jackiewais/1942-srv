#ifndef MESSAGES_H
#define MESSAGES_H

#include "../Parser/Parser.h"

class Elemento;

/* estructura paquete
 *
 * length[3] + cantidadDeMensajes[2] + mensaje1 +...
 *
*/

const int lengthl = 3,
	nOfMsgsl = 2,
	typel = 1,
	idl = 2,
	posl = 4,
	infol = 1,
	pathl = 60,
	idObjl = 9;

/* game standard message
 *
 * estructuras:
 *
 * tipo 1 - Agregar elemento
 * type
 * id
 * posx
 * posy
 *
 * tipo 2 - Actualizar elemento
 * type
 * id
 * posx
 * posy
 * info(estado)
 * 		a = alive
 * 		d = dead
 * 		s = shooting
 * 		c = connection problem
 *
 * tipo 3 - Eliminar elemento
 * type
 * id
 *
 * tipo 8 - Control de flujo
 * type
 * id
 * info (comandos)
 * 		s = start
 * 		r = restart
 * 		p = pause
 *
 */


struct gst {

	char type[typel +1];
	char id[idl];
	char ancho[posl];
	char alto[posl];
	char posx[posl];
	char posy[posl];
	char info[infol +1];
	char path[pathl];

};

enum msgType:char{

	UPDATE = '2',
	REMOVE = '3',
	CONTROL = '8',
	OBJETO = '0'

};

enum command:char{
	CON_SUCCESS = 's',
	CON_FAIL = 'f',
	DISCONNECT = 'q',
	REQ_SCENARIO = 'e'
};

/* En *msgs crea un array de punteros a gst's. Se accede a los gst como *((*msgs)[n])
 * Devuelve la cantidad de gst's creados
 */
int decodeMessages(struct gst*** msgs, char* msgsChar);

int encodeMessages(char** msgsChar, struct gst** msgs, int qty);

struct gst* genUpdateGstFromElemento(Elemento*);

struct gst* genAdminGst(int clientId, command comando);

struct gst* genGstFromCant(int cant);

struct gst* genGstFromFondo(Parser::type_Escenario * escenario, char path[pathl]);

struct gst* genGstFromElemento(Parser::type_Elemento * elem, char path[pathl]);

struct gst* genGstFromVentana(Parser::type_Ventana* ventana);


#endif

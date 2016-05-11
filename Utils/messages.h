#ifndef MESSAGES_H
#define MESSAGES_H

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
	infol = 1;

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
	char id[idl+1];
	char posx[posl+1];
	char posy[posl+1];
	char info[infol +1];

};

enum msgType:char{

	UPDATE = '2',
	REMOVE = '3',
	CONTROL = '8'

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


#endif

#ifndef PROTOCOLO_H_
#define PROTOCOLO_H_

//Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define VALOR_EXIT 1
#define FALLO_ENVIAR_MENSAJE "No se pudo enviar mensaje a traves del socket"
#define LARGO_MAXIMO_CLAVE 40

//HEADER
typedef enum{
	/*Headers utilizados para conexiones*/
	CONEXION, //Un cliente informa a un servidor que se ha conectado. Payload: Algun t_cliente
	FALLO_AL_RECIBIR,//Indica que un mensaje no se recibio correctamente en prot_recibir_mensaje
	DESCONEXION, //Indica que un cliente se ha desconectado
	ENVIO_DATOS,


	//Compartidos del request select
	FUNCION_SELECT,
	FUNCION_INSERT

} t_header;

typedef struct{
	t_header head;
	size_t tamanio_total;
	void* payload;
} t_prot_mensaje;

typedef enum{
	KERNEL,
	MEMORIA
} t_cliente;

/**
* @NAME: prot_recibir_mensaje
* @DESC: retorna un mensaje recibido en el socket.
* 		 El mensaje tiene la siguiente forma: HEADER + PAYLOAD
* 		 Quien recibe el mensaje debe castearlo asi:
* 		 t_prot_mensaje* mensaje_recibido = prot_recibir_mensaje(un_socket);
* 		 t_header header_recibido = mensaje_recibido->head;
* 		 t_algo payload = *(t_algo*) mensaje_recibido->payload;
*
* @PARAMS:
* 		socket_origen - el nombre lo dice...
*/
t_prot_mensaje* prot_recibir_mensaje(int socket_origen);

/**
* @NAME: mensaje_error_al_recibir
* @DESC: retorno de un prot_recibir_mensaje si hay un error al recibir
*/
t_prot_mensaje* mensaje_error_al_recibir();

/**
* @NAME: mensaje_desconexion_al_recibir
* @DESC: retorno de un prot_recibir_mensaje si hay desconexion
*/
t_prot_mensaje* mensaje_desconexion_al_recibir();

/**
* @NAME: prot_enviar_mensaje
* @DESC: funcion SUPER GENERICA que resume todas las anteriores
* 		 Basicamente, le pones el socket destino, el HEADER que queres mandar, el sizeof del payload, el puntero al payload (osea, lo pasas con &) y te hace todo el trabajo solo
*		 Ejemplo: quiero mandar un int, con el header NUMERO -supongamos que existe
*		 int a = 8
*		 prot_enviar_mensaje( destino, NUMERO, sizeof(int), &a);
* @PARAMS
* 		socket_destino - el nombre lo dice...
* 		header - el header
* 		tamanio_payload - el sizeof(tipo_de_lo_que_quiero_enviar)
* 		payload - &cosa_que_quiero_enviar
*/
void prot_enviar_mensaje(int socket_destino,t_header header , size_t tamanio_payload, void* payload);

//Hace los free
void prot_destruir_mensaje(t_prot_mensaje* victima);

/*Para cuando el payload es un string simple.
 * Lo lee y retorna el char* al mensaje
 * Hace el malloc dentro*/
char* leer_string_de_mensaje(t_prot_mensaje* mensaje);

#endif /* PROTOCOLO_H_ */

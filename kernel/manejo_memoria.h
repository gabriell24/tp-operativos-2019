
#ifndef MANEJO_MEMORIA_H_
#define MANEJO_MEMORIA_H_

//Includes
#include <stdio.h>
#include <stdlib.h>
#include "configuracion.h"
#include "consola.h"
#include "kernel.h"
#include "../shared_library/conexiones.h"
#include "../shared_library/protocolo.h"

//Defines

//Variables
//int socket_memoria;

//Prototipos
void conectar_a_memoria(char *ip, int puerto);
void recibir_mensajes_de_memoria();


#endif /* MANEJO_MEMORIA_H_ */

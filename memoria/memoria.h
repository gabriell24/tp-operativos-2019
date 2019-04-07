
#ifndef MEMORIA_H_
#define MEMORIA_H_

//Includes
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include "configuracion.h"
#include "consola.h"
#include "../shared_library/conexiones.h"
#include "../shared_library/protocolo.h"

//Defines
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * (EVENT_SIZE + 18) )

//Variables globales
pthread_t hilo_observer_configs;
pthread_t hilo_consola;
int socket_servidor;

//Prototipos
void printear_configuraciones();
void escuchar_cambios_en_configuraciones(void *);
void atender_memoria(int socket_servidor);

#endif /* MEMORIA_H_ */

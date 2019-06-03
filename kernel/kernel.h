
#ifndef KERNEL_H_
#define KERNEL_H_

//Includes
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include "configuracion.h"
#include "consola.h"
#include "manejo_memoria.h"
#include "criterios.h"

//Defines
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * (EVENT_SIZE + 18) )

//Variables globales
int socket_servidor;
pthread_t hilo_observer_configs;
pthread_t hilo_consola;
pthread_t hilo_manejo_memorias;
t_list *lista_sc;
t_list *lista_shc;
t_list *lista_ec;
//Prototipos
void printear_configuraciones();
void escuchar_cambios_en_configuraciones();

#endif /* KERNEL_H_ */

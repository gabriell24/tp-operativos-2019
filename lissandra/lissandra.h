#ifndef LISSANDRA_H_
#define LISSANDRA_H_

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
#include <commons/collections/list.h>
#include "configuracion.h"
#include "consola.h"

//Defines
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * (EVENT_SIZE + 18) )

//Variables globales
pthread_t hilo_observer_configs;
pthread_t hilo_consola;

typedef struct {
	int timestamp;
	uint16_t key;
	char *value;
} t_registro;

typedef struct {
	char *tabla;
	t_list *t_registro;
} t_memtable;

t_list *t_list_memtable;

//Prototipos
void printear_configuraciones();
void escuchar_cambios_en_configuraciones();
void cargar_datos_fake();

#endif /* LISSANDRA_H_ */

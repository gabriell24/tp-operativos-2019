
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
#include <pthread.h>
#include "configuracion.h"
#include "consola.h"
#include "../shared_library/conexiones.h"
#include "../shared_library/protocolo.h"
#include "../shared_library/estructuras_compartidas.h"

//Defines
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * (EVENT_SIZE + 18) )

//Variables globales
pthread_t hilo_observer_configs;
pthread_t hilo_consola;
pthread_t hilo_aceptar_clientes;
pthread_mutex_t mutex_journaling;
int socket_servidor;
int tamanio_value;
int tamanio_de_pagina;
t_list *tdp;
t_list *tds;
void *memoria;

typedef struct {
	char *nombre_segmento;
	t_list *segmentos;
} t_est_tds;

typedef struct {
	int nro_pag;
	void* ptr_posicion;
	uint8_t modificado;
} t_est_tdp;

/*typedef struct {
	int timestamp;
	uint16_t key;
	char *value;
} t_est_pag;*/

//Prototipos
void printear_configuraciones();
void iniciar_memoria();
int recibir_datos_de_fs(int socket);
void escuchar_cambios_en_configuraciones(void *);
void aceptar_clientes(int *socket_servidor);
void escuchar_kernel(int *socket_kernel);

#endif /* MEMORIA_H_ */

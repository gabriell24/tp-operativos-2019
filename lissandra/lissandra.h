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
#include "filesystem.h"
#include <signal.h>
#include "../shared_library/conexiones.h"
#include "../shared_library/protocolo.h"
#include "../shared_library/estructuras_compartidas.h"

//Defines
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * (EVENT_SIZE + 18) )

//Variables globales
pthread_t hilo_observer_configs;
pthread_t hilo_consola;
pthread_t hilo_conexion_memoria;
pthread_t hilo_dump;
pthread_mutex_t mutex_compactacion;
pthread_mutex_t mutex_rename_tmp;
pthread_mutex_t mutex_memtable;
pthread_mutex_t mutex_insert;
int maximo_caracteres_linea;
t_list *t_list_memtable;
int socket_servidor;

//Prototipos
void iniciar_semaforos();
void destruir_semaforos();
void aceptar_conexion_de_memoria(int *ptr_socket_servidor);
void printear_configuraciones();
void escuchar_cambios_en_configuraciones();
void escuchar_memoria(int *ptr_socket_cliente);
void cargar_datos_fake();

#endif /* LISSANDRA_H_ */

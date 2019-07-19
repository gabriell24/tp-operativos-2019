
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
#include <commons/bitarray.h>
#include <pthread.h>
#include "configuracion.h"
#include "consola.h"
#include "../shared_library/conexiones.h"
#include "../shared_library/protocolo.h"
#include "../shared_library/estructuras_compartidas.h"
#include "../shared_library/utiles.h"
#include "conexion_memoria.h"

//Defines
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * (EVENT_SIZE + 18) )
#define ERROR_NO_EXISTE_TABLA "NO_EXISTE_TABLA"
#define ERROR_KEY_NO_ENCONTRADA "KEY_NO_ENCONTRADA"

//Variables globales
pthread_t hilo_observer_configs;
pthread_t hilo_consola;
pthread_t hilo_aceptar_clientes;
pthread_t hilo_gossip;
pthread_t recibir_mensajes_de_kernel;
pthread_t hilo_journal;
pthread_mutex_t mutex_journaling;
int socket_servidor;
int tamanio_value;
int tamanio_de_pagina;
int socket_lissandra;
t_list *frames;
t_list *tds;
t_list *tabla_gossip;
void *memoria;
//char *bitmap;
//t_bitarray *estado_frames;

typedef struct {
	char *nombre_segmento;
	t_list *paginas;
} t_est_tds;

typedef struct {
	int nro_pag;
	void* ptr_posicion;
	int8_t modificado;
	uint64_t ultima_referencia;
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
t_est_tds *obtener_segmento_por_tabla(char *tabla);
t_est_tdp *obtener_pagina_por_key(t_list *lista, uint16_t key);
t_est_tdp *obtener_frame_libre();
t_est_tdp *obtener_frame();
uint16_t obtener_key_de_pagina(void *frame);
uint64_t obtener_timestamp_de_pagina(void *frame);
char *obtener_value_de_pagina(void *frame);
void crear_asignar_segmento(bool es_insert, t_est_tds *segmento, t_est_tdp* frame_libre, char *tabla, uint64_t timestamp, uint16_t key, char *value);
void limpiar_segmento(t_est_tds *segmento);
void limpiar_memoria();
void settear_timestamp(void* frame, uint64_t time);
void settear_key(void* frame, uint16_t key);
void settear_value(void *frame, char* value);
void printear_memoria();
t_est_tdp *frame_desde_lru();
void cargar_tabla_gossip();
bool ya_se_conecto_a(char *ip, int puerto);
void iniciar_gossip();
void reiniciar_frame(t_est_tdp *frame);
void journal_automatico();
//void agregar_nuevos_a_seeds(int nuevos_seeds);
#endif /* MEMORIA_H_ */

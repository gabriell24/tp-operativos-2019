#ifndef ESTRUCTURAS_COMPARTIDAS_H_
#define ESTRUCTURAS_COMPARTIDAS_H_

//Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include "protocolo.h"
#include "utiles.h"

//Defines
typedef struct {
	char* tabla;
	uint16_t key;
} t_request_select;

typedef struct {
	char* nombre_tabla;
	uint16_t key;
	char* value;
	int epoch;
} t_request_insert;
//Variables


typedef struct {
	char* nombre_tabla;
	char* tipo_consistencia;
	int numero_particiones;
	int compaction_time;
} t_request_create;

typedef struct {
	char* tabla;
	criterio consistencia;
} t_response_describe;

typedef struct {
	int nombre; //Los nombres, son numéricos
	char *ip;
	int puerto;
	int socket; //Solo para kernel
} t_memoria_conectada;

//Prototipos
void* serializar_request_select(char*tabla, uint16_t key);
t_request_select *deserializar_request_select(t_prot_mensaje *msje);

void* serializar_request_insert(char* nombre_tabla, uint16_t key, char* value, int epoch);
t_request_insert *deserializar_request_insert(t_prot_mensaje *msje);

void* serializar_request_create(char* nombre_tabla, char* tipo_consistencia, int numero_particiones, int compaction_time);
t_request_create *deserializar_request_create(t_prot_mensaje *msje);

void *serializar_response_describe(size_t tamanio_del_buffer, t_list *tablas);
t_list *deserializar_response_describe(t_prot_mensaje *mensaje, t_log *logger);
void imprimir_datos_describe(t_list *tablas);

void intercambir_memorias_conectadas(t_list *una_lista, t_list *otra_lista);
void *serializar_tabla_gossip(size_t tamanio_del_buffer, t_list *tabla);
t_list *deserializar_tabla_gossip(t_prot_mensaje *mensaje, t_log *logger);

#endif


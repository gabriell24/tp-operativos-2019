#ifndef ESTRUCTURAS_COMPARTIDAS_H_
#define ESTRUCTURAS_COMPARTIDAS_H_

//Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "protocolo.h"

//Defines
typedef struct {
	char* tabla;
	char* key;
} t_request_select;

typedef struct {
	char* nombre_tabla;
	char* key;
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



//Prototipos
void* serializar_request_select(char *key, char*value);
t_request_select *deserializar_request_select(t_prot_mensaje *msje);

void* serializar_request_insert(char* nombre_tabla, char* key, char* value, int epoch);
t_request_insert *deserializar_request_insert(t_prot_mensaje *msje);

void* serializar_request_create(char* nombre_tabla, char* tipo_consistencia, int numero_particiones, int compaction_time);
t_request_create *deserializar_request_create(t_prot_mensaje *msje);


#endif


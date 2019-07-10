
#ifndef INTERFAZ_H_
#define INTERFAZ_H_

//Includes
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "manejo_memoria.h"
#include "parser.h"
#include "../shared_library/estructuras_compartidas.h"
#include "../shared_library/protocolo.h"
//Defines

//estructuras y variables a globales

//protipos

void imprimir_metricas();





//Prototipos
void kernel_insert(int socket_memoria, char* nombre_tabla, uint16_t key, char* value, uint64_t epoch);
void kernel_select(int socket_memoria, char *tabla, uint16_t key);
void kernel_create(int socket_memoria, char* nombre_tabla, char* tipo_consistencia, int numero_particiones, int compaction_time);
void kernel_describe(int socket_memoria, char* nombre_tabla);
void kernel_drop(int socket_memoria, char* nombre_tabla);
void kernel_journal();
void kernel_run(char *archivo);
void kernel_add(int numero_memoria, char *criterio);


#endif /* INTERFAZ_H_ */

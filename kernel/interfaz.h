
#ifndef INTERFAZ_H_
#define INTERFAZ_H_

//Includes
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "manejo_memoria.h"
#include "../shared_library/estructuras_compartidas.h"
//Defines

//estructuras y variables a globales

//protipos

void imprimir_metricas();





//Prototipos
void kernel_insert(char* nombre_tabla, uint16_t key, char* value, int epoch);
void kernel_select(char *tabla, uint16_t key);
void kernel_create(char* nombre_tabla, char* tipo_consistencia, int numero_particiones, int compaction_time);
void kernel_describe(char* nombre_tabla);
void kernel_drop(char* nombre_tabla);
void kernel_journal();


#endif /* INTERFAZ_H_ */

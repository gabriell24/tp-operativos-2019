
#ifndef INTERFAZ_H_
#define INTERFAZ_H_

//Includes
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//Defines

//estructuras y variables a globales

//protipos

void imprimir_metricas();





//Prototipos
void kernel_insert(char* nombre_tabla, char* key, char* value, int epoch);
void kernel_select(char *tabla, char *clave);
void kernel_create(char* nombre_tabla, char* tipo_consistencia, int numero_particiones, int compactation_time);
void kernel_describe();
void kernel_drop(char* nombre_tabla);

#endif /* INTERFAZ_H_ */

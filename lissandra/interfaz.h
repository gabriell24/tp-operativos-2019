
#ifndef INTERFAZ_H_
#define INTERFAZ_H_

//Includes
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "consola.h"
//Defines

//Variables

//Prototipos
void fs_select(char *tabla, char *clave);
void fs_insert(char *tabla, char *clave, char *value, int timestamp);
void fs_create(char *tabla, char *tipo_consistencia, int particiones, int tiempo_compactacion);
void fs_describe(char *tabla);
void fs_drop(char *tabla);

#endif /* INTERFAZ_H_ */

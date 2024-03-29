
#ifndef INTERFAZ_H_
#define INTERFAZ_H_

//Includes
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "consola.h"
#include "filesystem.h"
#include "lissandra.h"
#include "compactar.h"
#include <commons/string.h>
//Defines
#define ERROR_NO_EXISTE_TABLA "NO_EXISTE_TABLA"
#define ERROR_KEY_NO_ENCONTRADA "KEY_NO_ENCONTRADA"

//Variables

//Prototipos
char *fs_select(char *tabla, uint16_t key);
void fs_insert(char *tabla, uint16_t key, char *value, uint64_t timestamp);
char *fs_create(char *tabla, char *tipo_consistencia, int particiones, int tiempo_compactacion);
t_list *fs_describe(char *tabla);
bool fs_drop(char *tabla);

#endif /* INTERFAZ_H_ */

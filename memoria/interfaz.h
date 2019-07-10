
#ifndef INTERFAZ_H_
#define INTERFAZ_H_

#include <stdlib.h>
#include <stdint.h>
#include "memoria.h"


char *memoria_select(char *tabla, uint16_t key);
void memoria_insert(char *tabla, uint16_t key, char *value, uint64_t timestamp);
void memoria_create(char *tabla, char *consistencia, int particiones, int compaction_time);
void memoria_describe(char *tabla);
void memoria_drop(char *tabla);
void journal();

#endif /* INTERFAZ_H_ */

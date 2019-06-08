
#ifndef DUMP_H_
#define DUMP_H_

#include <stdio.h>
#include "configuracion.h"
#include <math.h>
#include <stdlib.h>
#include "lissandra.h"
#include "filesystem.h"


void dump_automatico();
void dumpear();
void crear_archivo_temporal(char *tabla, int size, char *datos);
char *nombre_basado_en_temporales(char *tabla, char *ruta_tabla);
#endif /* DUMP_H_ */

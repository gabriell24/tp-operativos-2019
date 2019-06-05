
#ifndef DUMP_H_
#define DUMP_H_

#include <stdio.h>
#include "configuracion.h"
#include "lissandra.h"


void dump_automatico();
void dumpear();
FILE *crear_y_devolver_archivo_temporal(char *tabla);
char *nombre_basado_en_temporales(char *tabla, char *ruta_tabla);
#endif /* DUMP_H_ */

#ifndef COMPACTAR_H_
#define COMPACTAR_H_

//Includes
#include <stdio.h>
#include "configuracion.h"
#include <math.h>
#include <stdlib.h>
#include "lissandra.h"
#include "filesystem.h"

//Prototipos
void crear_archivo_tmpc(char *tabla, int size, char *datos);
void compactar();
t_list *limpiar_lista_de_duplicados(t_list *lineas_a_compactar, t_list *lineas_leidas);
char *nombre_basado_en_tmpc(char *tabla, char *ruta_tabla);

#endif

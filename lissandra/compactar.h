#ifndef COMPACTAR_H_
#define COMPACTAR_H_

//Includes
#include <stdio.h>
#include "configuracion.h"
#include <math.h>
#include <stdlib.h>
#include "lissandra.h"
#include "filesystem.h"
#include <commons/collections/list.h>

typedef struct {
	char *nombre;
	pthread_t tid; //Id del hilo
} t_tabla_compactacion;
t_list *tablas_en_compactacion;
pthread_mutex_t mutex_lista_compactacion;

//Prototipos
void crear_archivo_tmpc(char *tabla, int size, char *datos);
void compactar(char *tabla);
uint32_t get_tiempo_compactacion(char *unaTabla);
void efectuar_compactacion(char *unaTabla);
t_list *limpiar_lista_de_duplicados(t_list *lineas_a_compactar, t_list *lineas_leidas);
char *nombre_basado_en_tmpc(char *tabla, char *ruta_tabla);

//Nuevos
void agregar_tabla_a_compactar(char *tabla);
t_tabla_compactacion *obtener_tabla_compactacion(char *tabla);
void cancelar_hilo_compactacion(char *tabla);
void liberar_registro_tabla_compactacion(t_tabla_compactacion *tabla);
void limpiar_lista_compactacion();

#endif

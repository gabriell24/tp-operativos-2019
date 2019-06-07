
#ifndef PLANIFICACION_H_
#define PLANIFICACION_H_

#include <stdlib.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "kernel.h"

t_list *lista_nuevos;
t_list *lista_ready;
t_list *lista_exec;
pthread_t hilo_admision;
sem_t lqls_en_ready;
sem_t instancias_exec;
pthread_mutex_t mutex_listas;

typedef struct {
	int prox_linea_ejecutar;
	t_list *lineas;
	int lineas_a_ejecutar;
} t_lql;

typedef struct {
	char *data;
	bool desde_consola;
	t_lql *lql;
} compartir_info_lql;

typedef struct {
	uint8_t posicion;
	bool ocupada;
} estado_exec;

void iniciar_listas_planificacion();
void admitir_lql(compartir_info_lql *datos);
t_lql *lql_desde_consola(char *linea);
t_lql *lql_desde_archivo(char *path_archivo);
void crear_un_lql(bool desde_consola, char *data);
void planificar();
int obtener_exec_libre();
void corte_quantum(int nro_exec);
void round_robin();
void exit_lql(t_lql *lql);

#endif /* PLANIFICACION_H_ */

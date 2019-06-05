
#ifndef PLANIFICACION_H_
#define PLANIFICACION_H_

#include "kernel.h"
#include <commons/string.h>
#include <commons/collections/list.h>

t_list *lista_nuevos;
t_list *lista_ready;
t_list *lista_exec;

typedef struct {
	int prox_linea_ejecutar;
	char **lineas;
} t_pcb;

#endif /* PLANIFICACION_H_ */


#ifndef CONEXION_MEMORIA_H_
#define CONEXION_MEMORIA_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include "consola.h"


void escuchar_memoria(int *socket);
void liberar_tabla_gossip(t_list *tabla);
#endif /* CONEXION_MEMORIA_H_ */

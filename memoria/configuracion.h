
#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>

typedef struct {
	char *ip_servidor;
	int puerto_escucha;
	char *ip_lissandra;
	int puerto_lissandra;
	char** ip_seeds;
	char** puerto_seeds;
	int retardo_accesso_a_mp;
	int retardo_accesso_a_fs;
	int tamanio_de_memoria;
	int tiempo_journaling;
	int tiempo_gossiping;
	int numero_memoria;
	bool en_produccion;
} t_memoria_config;

t_config *configuracion;
t_memoria_config memoria_config;

void levantar_archivo_configuracion();
void limpiar_configuraciones();
#endif /* CONFIGURACION_H_ */

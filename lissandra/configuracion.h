
#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

#include <commons/config.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
	int puerto_escucha;
	char *punto_montaje;
	int retardo_ms;
	int tamanio_value;
	int tiempo_dump_ms;
	bool en_produccion;
} t_fs_config;

typedef struct {
	int timestamp;
	char *value;
} t_timestamp_value;

t_config *configuracion;
t_fs_config fs_config;

void levantar_archivo_configuracion();
#endif /* CONFIGURACION_H_ */

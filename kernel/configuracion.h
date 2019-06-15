
#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

//Includes
#include <stdbool.h>
#include <string.h>
#include <commons/config.h>

//Defines

//Variables
typedef struct {
	char *ip_memoria;
	int puerto_memoria;
	int quantum;
	int multiprocesamiento;
	int refrescar_metadata;
	int retardo_ciclo_ejecucion;
	bool en_produccion;
} t_kernel_config;

t_config *configuracion;
t_kernel_config kernel_config;

//Prototipo
void levantar_archivo_configuracion();
void recargar_archivo_configuracion();
#endif /* CONFIGURACION_H_ */

#include "configuracion.h"

void levantar_archivo_configuracion()
{
	configuracion = config_create("kernel.config");

	kernel_config.ip_memoria = strdup(config_get_string_value(configuracion, "IP_MEMORIA"));
	kernel_config.puerto_memoria = config_get_int_value(configuracion, "PUERTO_MEMORIA");
	kernel_config.quantum = config_get_int_value(configuracion, "QUANTUM");
	kernel_config.multiprocesamiento = config_get_int_value(configuracion, "MULTIPROCESAMIENTO");
	kernel_config.refrescar_metadata = config_get_int_value(configuracion, "REFRESCAR_METADATA");
	kernel_config.retardo_ciclo_ejecucion = config_get_int_value(configuracion, "RETARDO_CICLO_EJECUCION");
	kernel_config.en_produccion = (bool)config_get_int_value(configuracion, "EN_PRODUCCION");

	config_destroy(configuracion);
}

void recargar_archivo_configuracion()
{
	configuracion = config_create("kernel.config");

	kernel_config.quantum = config_get_int_value(configuracion, "QUANTUM");
	kernel_config.refrescar_metadata = config_get_int_value(configuracion, "REFRESCAR_METADATA");
	kernel_config.retardo_ciclo_ejecucion = config_get_int_value(configuracion, "RETARDO_CICLO_EJECUCION");

	config_destroy(configuracion);
}

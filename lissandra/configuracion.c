#include "configuracion.h"

void levantar_archivo_configuracion()
{
	configuracion = config_create("lissandra.config");

	fs_config.puerto_escucha = config_get_int_value(configuracion, "PUERTO_ESCUCHA");
	fs_config.punto_montaje = strdup(config_get_string_value(configuracion, "PUNTO_MONTAJE"));
	fs_config.retardo_ms = config_get_int_value(configuracion, "RETARDO");
	fs_config.tamanio_value = config_get_int_value(configuracion, "TAMANIO_VALUE");
	fs_config.en_produccion = (bool)config_get_int_value(configuracion, "EN_PRODUCCION");
	fs_config.tiempo_dump_ms = config_get_int_value(configuracion, "TIEMPO_DUMP");

	config_destroy(configuracion);
}

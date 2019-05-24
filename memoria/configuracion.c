#include "configuracion.h"

void levantar_archivo_configuracion()
{
	configuracion = config_create("memoria.config");

	memoria_config.puerto_escucha = config_get_int_value(configuracion, "PUERTO_ESCUCHA" );
	memoria_config.ip_lissandra = strdup(config_get_string_value(configuracion, "IP_LISSANDRA"));
	memoria_config.puerto_lissandra = config_get_int_value(configuracion, "PUERTO_LISSANDRA");
	memoria_config.ip_seeds = config_get_array_value(configuracion, "IP_SEEDS");
	memoria_config.puerto_seeds = config_get_array_value(configuracion, "PUERTO_DE_SEEDS");
	memoria_config.retardo_accesso_a_mp = config_get_int_value(configuracion, "RETARDO_ACCESO_A_MP");
	memoria_config.retardo_accesso_a_fs = config_get_int_value(configuracion, "RETARDO_ACCESO_A_FS");
	memoria_config.tamanio_de_memoria = config_get_int_value(configuracion, "TAMANIO_DE_MEMORIA");
	memoria_config.tiempo_journaling = config_get_int_value(configuracion, "TIEMPO_DE_JOURNAL");
	memoria_config.tiempo_gossiping = config_get_int_value(configuracion, "TIEMPO_DE_GOSSIPING");
	memoria_config.numero_memoria = config_get_int_value(configuracion, "NUMERO_DE_MEMORIA");
	memoria_config.en_produccion  = (bool)config_get_int_value(configuracion, "EN_PRODUCCION");

	config_destroy(configuracion);
}

void limpiar_configuraciones() {
	free(memoria_config.ip_lissandra);
	string_iterate_lines(memoria_config.ip_seeds, (void*)free);
	free(memoria_config.ip_seeds);
	string_iterate_lines(memoria_config.puerto_seeds, (void*)free);
	free(memoria_config.puerto_seeds);
}

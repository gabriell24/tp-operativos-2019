#include "interfaz.h"

void fs_select(char *tabla, uint16_t key) {
	if(!existe_tabla(tabla)) {
		log_error(logger, "[CREATE] ERROR: No existe una tabla con ese nombre.");
		return;
	}
	char * ruta = path_tablas();
	string_append_with_format(&ruta,"%s/Metadata",tabla);
	t_config *conf = config_create(ruta);
	int particiones = config_get_int_value(conf,"PARTITIONS");
	printf ("PARTITIONS%d\n",particiones);
	int particion_a_leer = calcular_particion(particiones,key);
	printf ("Calcule la partición %d\n",particion_a_leer);
	config_destroy(conf);
	free (ruta);
}

void fs_insert(char *tabla, uint16_t key, char *value, int timestamp) {
	log_info(logger, "[EPOCH] timestamp: %d", timestamp);
}

void fs_create(char *tabla, char *tipo_consistencia, int particiones, int tiempo_compactacion) {
	if(existe_tabla(tabla)) {
		log_error(logger, "[CREATE] ERROR: Ya existe tabla con ese nombre.");
		return;
	}
	crear_carpeta_tabla(tabla);
	guardar_archivo_metadata(tabla, tipo_consistencia, particiones, tiempo_compactacion);
}

void fs_describe(char *tabla) {
	bool mostrar_todo = tabla == NULL;
	//Dummy
}

void fs_drop(char *tabla) {
	//Dummy
}


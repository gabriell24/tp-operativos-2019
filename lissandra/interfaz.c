#include "interfaz.h"

char *fs_select(char *tabla, uint16_t key) {
	if(!existe_tabla(tabla)) {
		log_error(logger, "[CREATE] ERROR: No existe una tabla con ese nombre.");
		return ERROR_NO_EXISTE_TABLA;
	}
	char *ruta = path_tablas();
	string_append_with_format(&ruta,"%s/Metadata",tabla);
	t_config *conf = config_create(ruta);
	int particiones = config_get_int_value(conf,"PARTITIONS");
	config_destroy(conf);
	free (ruta);

	//printf ("PARTITIONS%d\n",particiones);
	int particion_a_leer = calcular_particion(particiones,key);
	//printf ("Calcule la partición %d\n",particion_a_leer);
	char *path_a_particion = string_new();
	string_append_with_format(&path_a_particion,"%s/%d.bin",tabla, particion_a_leer);
	//printf ("Calcule la partición %s\n",path_a_particion);
	//char *linea_con_la_key = string_new();
	return obtener_datos(path_a_particion, key);

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
	for(int b=0; b<particiones;b++){
		int bloque_asignado = tomar_bloque_libre();
		if(bloque_asignado == -1){
			log_error(logger, "[CREATE] ERROR: No hay mas bloques libres.");
		}
		crear_archivo_bloque(bloque_asignado, "");
	}
}

void fs_describe(char *tabla) {
	bool mostrar_todo = tabla == NULL;
	//Dummy
}

void fs_drop(char *tabla) {
	//Dummy
}


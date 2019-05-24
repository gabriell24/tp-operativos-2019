#include "interfaz.h"

char *fs_select(char *tabla, uint16_t key) {
	if(!existe_tabla(tabla)) {
		log_error(logger, "[SELECT] ERROR: No existe una tabla con ese nombre.");
		return ERROR_NO_EXISTE_TABLA;
	}
	t_metadata metadata = obtener_metadata(tabla);

	//printf ("PARTITIONS%d\n",particiones);
	int particion_a_leer = calcular_particion(metadata.partitions, key);
	free(metadata.consistency);
	//printf ("Calcule la partición %d\n",particion_a_leer);
	char *path_a_particion = string_new();
	string_append_with_format(&path_a_particion,"%s/%d.bin",tabla, particion_a_leer);
	//printf ("Calcule la partición %s\n",path_a_particion);
	//char *linea_con_la_key = string_new();
	return obtener_datos(path_a_particion, key);

}

void fs_insert(char *tabla, uint16_t key, char *value, int timestamp) {
	if(!existe_tabla(tabla)) {
		log_error(logger, "[INSERT] ERROR: No existe una tabla con ese nombre.");
		return;
	}
	log_info(logger, "[EPOCH] timestamp: %d", timestamp);
	if(strlen(value) > fs_config.tamanio_value) {
		log_error(logger, "[Error] El value no puede superar %d caracteres", fs_config.tamanio_value);
	}
}

void fs_create(char *tabla, char *tipo_consistencia, int particiones, int tiempo_compactacion) {
	if(existe_tabla(tabla)) {
		log_error(logger, "[CREATE] ERROR: Ya existe tabla con ese nombre.");
		return;
	}
	string_to_upper(tipo_consistencia);
	if(!string_equals_ignore_case(tipo_consistencia, "SC") && !string_equals_ignore_case(tipo_consistencia, "SHC") && !string_equals_ignore_case(tipo_consistencia, "EC")) {
		log_error(logger, "[Error] Consistencia no reconocida");
		return;
	}
	if(particiones < 1 || tiempo_compactacion < 1) {
		log_error(logger, "[Error] Los tiempos y/o cantidades deben ser mayores a cero");
		return;
	}
	int bloques_necesarios[particiones];
	for(int indice_bloque = 0; indice_bloque < particiones; indice_bloque++){
		bloques_necesarios[indice_bloque] = tomar_bloque_libre();
		if(bloques_necesarios[indice_bloque] == -1){
			log_error(logger, "[CREATE] ERROR: No hay más bloques libres. No voy a crear tu tabla, limpio los solicitados");
			for(int base = 0; base < indice_bloque; base++) {
				bitarray_clean_bit(datos_fs.bitarray, bloques_necesarios[base]);
			}
			return;
		}
	}

	crear_carpeta_tabla(tabla);
	guardar_archivo_metadata(tabla, tipo_consistencia, particiones, tiempo_compactacion);
	for(int b=0; b<particiones;b++){
		crear_archivo_particion(tabla, b, bloques_necesarios[b]);
		crear_archivo_bloque(bloques_necesarios[b], "");

	}
}

void fs_describe(char *tabla) {
	bool mostrar_todo = tabla == NULL;
	if(mostrar_todo) {

	} else {
		char *path = string_new();
		string_append_with_format(&path, "%s%s/%s", path_tablas(), tabla, "Metadata");
	}
	//Dummy
}

void fs_drop(char *tabla) {
	//Dummy
}


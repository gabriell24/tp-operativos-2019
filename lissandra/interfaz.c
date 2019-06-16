#include "interfaz.h"

char *fs_select(char *tabla, uint16_t key) {
	if(!existe_tabla(tabla)) {
		log_error(logger, "[SELECT] ERROR: No existe una tabla con ese nombre.");
		return ERROR_NO_EXISTE_TABLA;
	}
	t_metadata metadata = obtener_metadata(tabla);
	char *retorno = NULL;
	t_list *key_encontradas = list_create();
	//t_timestamp_value *desde_particion = NULL;
	//t_timestamp_value *desde_memtable = NULL;
	//t_timestamp_value *desde_temporal = NULL;
	//t_timestamp_value *desde_temporal_compactacion = NULL;
	t_timestamp_value *mayor_timestamp = NULL;

	int particion_a_leer = calcular_particion(metadata.partitions, key);
	char *path_a_particion = string_new();
	string_append_with_format(&path_a_particion,"%s/%d.bin",tabla, particion_a_leer);
	list_add_all(key_encontradas, obtener_datos_de_particion(path_a_particion, key));
	//t_registro *datos_memtable;
	/*if((datos_memtable = obtener_registros_por_key(tabla, key))) {
		desde_memtable = malloc(sizeof(t_timestamp_value));
		desde_memtable->timestamp = datos_memtable->timestamp;
		desde_memtable->value = string_duplicate(datos_memtable->value);
	}*/

	list_add_all(key_encontradas, obtener_registros_por_key(tabla, key));
	/*
	 * BUSCAR EN ARCHIVOS TEMPORALES
	 * Y ASGINAR A desde_temporal
	 */
	//char *path_a_temporal = string_new();
	//bool reviso_temporales = false;
	//char *nombre = string_new();
	//string_append(&nombre, tabla);
	char *ruta_tabla = path_tablas();
	string_append(&ruta_tabla, tabla);
	DIR *dp;
	struct dirent *ep;
	dp = opendir(ruta_tabla);
	if (dp != NULL) {
		while ((ep = readdir (dp))) {
			if(string_ends_with(ep->d_name, ".tmp")) {
				char *archivo_temporal = string_from_format("%s/%s", tabla, ep->d_name);
				list_add_all(key_encontradas, obtener_datos_de_particion(archivo_temporal, key));
				free(archivo_temporal);

			}

			if(string_ends_with(ep->d_name, ".tmpc")) {
				char *archivo_temporal = string_from_format("%s/%s", tabla, ep->d_name);
				list_add_all(key_encontradas, obtener_datos_de_particion(archivo_temporal, key));
				free(archivo_temporal);
			}
		}
	}
	closedir(dp);
	free(ruta_tabla);

	mayor_timestamp = devolver_timestamp_mayor(key_encontradas);
	if(!mayor_timestamp) {
		return ERROR_KEY_NO_ENCONTRADA;
	} else {
		char *timestamp_key = string_from_format("%d;%d;", mayor_timestamp->timestamp, key);
		size_t tamanio_del_stream = strlen(timestamp_key)+strlen(mayor_timestamp->value);
		retorno = malloc(tamanio_del_stream+1);
		memset(retorno, 0, tamanio_del_stream+1);
		memcpy(retorno, timestamp_key, strlen(timestamp_key));
		memcpy(retorno+strlen(timestamp_key), mayor_timestamp->value, strlen(mayor_timestamp->value));
		free(timestamp_key);
	}
	//limpiar_timestampvalue_si_corresponde(desde_particion);
	//limpiar_timestampvalue_si_corresponde(desde_memtable);
	//limpiar_timestampvalue_si_corresponde(desde_temporal);
	//mayor_timestamp no se limpiaria, porque es uno de los 3 anteriores
	list_destroy_and_destroy_elements(key_encontradas, (void*)free);
	free(metadata.consistency);

	return retorno;

}

void fs_insert(char *tabla, uint16_t key, char *value, int timestamp) {
	if(!existe_tabla(tabla)) {
		log_error(logger, "[INSERT] ERROR: No existe una tabla con ese nombre.");
		return;
	}
	if(strlen(value) > fs_config.tamanio_value) {
		log_error(logger, "[Error] El value no puede superar %d caracteres", fs_config.tamanio_value);
		return;
	}
	log_info(logger, "[EPOCH] timestamp: %d", timestamp);

	//TODO NO entedí el item 3 del enunciado

	t_registro *unRegistro = malloc(sizeof(t_registro));
	unRegistro->key = key;
	unRegistro->timestamp = timestamp;
	unRegistro->value = string_duplicate(value);
	t_memtable *tabla_existente_en_memtable = obtener_tabla_en_memtable(tabla);

	if(tabla_existente_en_memtable) {
		log_debug(logger, "[MEMTABLE] Existia el area %s", tabla);
		list_add(tabla_existente_en_memtable->t_registro, unRegistro);
	}
	else {
		t_list *registros = list_create();
		list_add(registros, unRegistro);

		t_memtable *unaTabla = malloc(sizeof(t_memtable));
		unaTabla->tabla = strdup(tabla);
		unaTabla->t_registro = registros;

		log_debug(logger, "[MEMTABLE] Agrego el area %s", tabla);
		list_add(t_list_memtable, unaTabla);
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

t_list *fs_describe(char *tabla) {
	bool mostrar_todo = tabla == NULL;
	//int cantidad_de_tablas;
	t_list *metadatas = list_create();
	char *path = string_new();
	string_append_with_format(&path, "%s", path_tablas());
	if(mostrar_todo) {


		DIR *dp;
		struct dirent *ep;

		dp = opendir(path);
		if (dp != NULL) {
			while ((ep = readdir (dp))) {
				if(!string_contains(ep->d_name, ".")) {
					char *path_tabla = string_duplicate(path);
					string_append(&path_tabla, ep->d_name);
					log_info(logger, "[Describe] INCLUIDO: %s", ep->d_name);
					t_response_describe *describe = devolver_metadata(path_tabla, ep->d_name);
					list_add(metadatas, describe);
					free(path_tabla);
				}
				else {
					log_debug(logger, "[Describe] IGNORADO: %s", ep->d_name);
				}
			}

			closedir(dp);
		}
		else {
			perror ("No se pudo escanear los directorios");
		}
	} else {
		if(!existe_tabla(tabla)) {
			log_error(logger, "[SELECT] ERROR: No existe una tabla con ese nombre.");
			return NULL;
		}
		string_append(&path, tabla);
		t_response_describe *describe = devolver_metadata(path, tabla);
		list_add(metadatas, describe);

	}
	free(path);
	return metadatas;
}

void fs_drop(char *tabla) {
	//Dummy
}


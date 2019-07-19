#include "interfaz.h"

char *fs_select(char *tabla, uint16_t key) {
	pthread_mutex_lock(&mutex_compactacion);
	if(!existe_tabla(tabla)) {
		loguear(error, logger, "[SELECT] ERROR: No existe una tabla con ese nombre.");
		return ERROR_NO_EXISTE_TABLA;
	}
	t_metadata metadata = obtener_metadata(tabla);
	char *retorno = NULL;
	t_list *key_encontradas = list_create();
	t_timestamp_value *mayor_timestamp = NULL;

	int particion_a_leer = calcular_particion(metadata.partitions, key);
	char *path_a_particion = string_new();
	string_append_with_format(&path_a_particion,"%s/%d.bin",tabla, particion_a_leer);
	t_list *respuesta_busqueda = obtener_datos_de_particion(path_a_particion, key);
	list_add_all(key_encontradas, respuesta_busqueda);
	list_destroy(respuesta_busqueda);
	free(path_a_particion);

	respuesta_busqueda = obtener_registros_por_key(tabla, key);
	list_add_all(key_encontradas, respuesta_busqueda);
	list_destroy(respuesta_busqueda);

	char *ruta_tabla = path_tablas();
	string_append(&ruta_tabla, tabla);
	DIR *dp;
	struct dirent *ep;
	pthread_mutex_lock(&mutex_rename_tmp);
	dp = opendir(ruta_tabla);
	if (dp != NULL) {
		while ((ep = readdir (dp))) {
			//Este busca en el .tmp y en .tmpc
			if(string_contains(ep->d_name, ".tmp")) {
				char *archivo_temporal = string_from_format("%s/%s", tabla, ep->d_name);
				respuesta_busqueda = obtener_datos_de_particion(archivo_temporal, key);
				list_add_all(key_encontradas, respuesta_busqueda);
				list_destroy(respuesta_busqueda);
				free(archivo_temporal);
			}
		}
	}
	closedir(dp);
	pthread_mutex_unlock(&mutex_rename_tmp);
	free(ruta_tabla);

	mayor_timestamp = devolver_timestamp_mayor(key_encontradas);
	if(!mayor_timestamp) {
		list_destroy(key_encontradas);
		free(metadata.consistency);
		return ERROR_KEY_NO_ENCONTRADA;
	} else {
		char *timestamp_key = string_from_format("%llu;%d;", mayor_timestamp->timestamp, key);
		size_t tamanio_del_stream = strlen(timestamp_key)+strlen(mayor_timestamp->value);
		retorno = malloc(tamanio_del_stream+1);
		memset(retorno, 0, tamanio_del_stream+1);
		memcpy(retorno, timestamp_key, strlen(timestamp_key));
		memcpy(retorno+strlen(timestamp_key), mayor_timestamp->value, strlen(mayor_timestamp->value));
		free(timestamp_key);
		free(mayor_timestamp->value);
		free(mayor_timestamp);
	}

	list_destroy_and_destroy_elements(key_encontradas, (void*)free);
	free(metadata.consistency);
	pthread_mutex_unlock(&mutex_compactacion);
	return retorno;

}

void fs_insert(char *tabla, uint16_t key, char *value, uint64_t timestamp) {
	pthread_mutex_lock(&mutex_compactacion);
	if(!existe_tabla(tabla)) {
		loguear(error, logger, "[INSERT] ERROR: No existe una tabla con ese nombre.");
		return;
	}
	if(strlen(value) > fs_config.tamanio_value) {
		loguear(error, logger, "[Error] El value no puede superar %d caracteres", fs_config.tamanio_value);
		return;
	}
	loguear(info, logger, "[EPOCH] timestamp: %llu", timestamp);

	//TODO NO entedí el item 2 del enunciado

	t_registro *unRegistro = malloc(sizeof(t_registro));
	unRegistro->key = key;
	unRegistro->timestamp = timestamp;
	unRegistro->value = string_duplicate(value);
	t_memtable *tabla_existente_en_memtable = obtener_tabla_en_memtable(tabla);

	if(tabla_existente_en_memtable) {
		loguear(debug, logger, "[MEMTABLE] Existia el area %s", tabla);
		list_add(tabla_existente_en_memtable->t_registro, unRegistro);
	}
	else {
		loguear(info, logger, "Creo hilo para compactación");
		pthread_t hilo_dump_por_tabla;
		char *nombre_tabla = string_duplicate(tabla);
		pthread_create(&hilo_dump_por_tabla, NULL, (void *)compactar, nombre_tabla);
		t_list *registros = list_create();
		list_add(registros, unRegistro);

		t_memtable *unaTabla = malloc(sizeof(t_memtable));
		unaTabla->tabla = strdup(tabla);
		unaTabla->t_registro = registros;

		loguear(debug, logger, "[MEMTABLE] Agrego el area %s", tabla);
		list_add(t_list_memtable, unaTabla);
	}
	pthread_mutex_unlock(&mutex_compactacion);
}

char *fs_create(char *tabla, char *tipo_consistencia, int particiones, int tiempo_compactacion) {
	char *retorno = string_new();
	if(existe_tabla(tabla)) {
		loguear(error, logger, "[CREATE] ERROR: Ya existe tabla con ese nombre.");
		string_append(&retorno, "[Create] Tabla ya existia");
		return retorno;
	}
	string_to_upper(tipo_consistencia);
	if(!string_equals_ignore_case(tipo_consistencia, "SC") && !string_equals_ignore_case(tipo_consistencia, "SHC") && !string_equals_ignore_case(tipo_consistencia, "EC")) {
		loguear(error, logger, "[Error] Consistencia no reconocida");
		string_append(&retorno, "[Create] Consitencia no reconocida");
		return retorno;
	}
	if(particiones < 1 || tiempo_compactacion < 1) {
		loguear(error, logger, "[Error] Los tiempos y/o cantidades deben ser mayores a cero");
		string_append(&retorno, "[Create] Cantidades no naturales");
		return retorno;
	}
	int bloques_necesarios[particiones];
	for(int indice_bloque = 0; indice_bloque < particiones; indice_bloque++){
		bloques_necesarios[indice_bloque] = tomar_bloque_libre();
		if(bloques_necesarios[indice_bloque] == -1){
			loguear(error, logger, "[CREATE] ERROR: No hay más bloques libres. No voy a crear tu tabla, limpio los solicitados");
			for(int base = 0; base < indice_bloque; base++) {
				bitarray_clean_bit(datos_fs.bitarray, bloques_necesarios[base]);
			}
			string_append_with_format(&retorno, "[Create] No hay suficientes bloques libres");
			return retorno;
		}
	}

	crear_carpeta_tabla(tabla);
	guardar_archivo_metadata(tabla, tipo_consistencia, particiones, tiempo_compactacion);
	for(int b=0; b<particiones;b++){
		crear_archivo_particion(tabla, b, bloques_necesarios[b]);
		crear_archivo_bloque(bloques_necesarios[b], "");

	}
	/*TODO: Evaluar si el enunciado lo pide aca!!
	 * Evaluar hacer una funcion generica donde creo un hilo para compactar. Motivo: una tabla existente.
	t_list *registros = list_create();

	t_memtable *unaTabla = malloc(sizeof(t_memtable));
	unaTabla->tabla = strdup(tabla);
	unaTabla->t_registro = registros;

	list_add(t_list_memtable, unaTabla);
	*/
	string_append_with_format(&retorno, "[Create] Tabla creada %s", tabla);
	return retorno;
}

t_list *fs_describe(char *tabla) {
	bool mostrar_todo = tabla == NULL;
	//int cantidad_de_tablas;
	t_list *metadatas = list_create();
	char *path = path_tablas();
	if(mostrar_todo) {


		DIR *dp;
		struct dirent *ep;

		dp = opendir(path);
		if (dp != NULL) {
			while ((ep = readdir (dp))) {
				if(!string_contains(ep->d_name, ".")) {
					char *path_tabla = string_duplicate(path);
					string_append(&path_tabla, ep->d_name);
					loguear(info, logger, "[Describe] INCLUIDO: %s", ep->d_name);
					t_response_describe *describe = devolver_metadata(path_tabla, ep->d_name);
					list_add(metadatas, describe);
					free(path_tabla);
				}
				else {
					loguear(debug, logger, "[Describe] IGNORADO: %s", ep->d_name);
				}
			}

			closedir(dp);
		}
		else {
			perror ("No se pudo escanear los directorios");
		}
	} else {
		if(!existe_tabla(tabla)) {
			loguear(error, logger, "[SELECT] ERROR: No existe una tabla con ese nombre.");
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
	pthread_mutex_lock(&mutex_compactacion);

	//escribir aqui

	pthread_mutex_unlock(&mutex_compactacion);
}


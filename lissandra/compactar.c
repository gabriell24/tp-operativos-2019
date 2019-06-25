#include "compactar.h"

void compactar(char *tabla) {
	//TODO ver de que manera se pede cancelar el hilo, si le hacen drop a la tabla
	while(!consola_ejecuto_exit) {
		log_info(logger, "Reviso si %s necesita compactar", (char *)tabla);
		efectuar_compactacion((char *)tabla);
		log_info(logger, "Fin revisión para compactar en %s, duermo", (char *)tabla);
		usleep(fs_config.tiempo_dump_ms * 1000);
	}
	//free(tabla);
	//pthread_join(pthread_self());
}
void efectuar_compactacion(char *unaTabla) {
	char *path = string_new();

	char *path_tabla = path_tablas();
	string_append_with_format(&path, "%s%s/", path_tabla, unaTabla);

	DIR *dp;
	struct dirent *ep;

	t_list *bloques_que_uso = list_create();

	bool necesita_compactar = false;

	t_list *archivos_a_borrar = list_create();
	dp = opendir(path);
	if (dp != NULL) {
		while ((ep = readdir (dp))) { /*Renombro tmp por tmpc*/
			if(string_ends_with(ep->d_name, ".tmp")){
				necesita_compactar = true;
				char **separo_extension = string_split(ep->d_name, ".");
				char *nuevo_nombre_con_extension = string_from_format("%s.tmpc",  separo_extension[0]);
				char *path_fuente = string_from_format("%s%s", path, ep->d_name);
				char *path_destino = string_from_format("%s%s", path, nuevo_nombre_con_extension);
				rename(path_fuente, path_destino);
				list_add(archivos_a_borrar, string_duplicate(path_destino));
				t_config *temporal_config = config_create(path_destino);
					char **bloques_usados = config_get_array_value(temporal_config, "BLOCKS");
					int posicion = 0;
					while (bloques_usados[posicion] != NULL){
						list_add(bloques_que_uso, string_duplicate(bloques_usados[posicion]));
						posicion++;
					}
					string_iterate_lines(bloques_usados, (void *)free);
					free(bloques_usados);
				config_destroy(temporal_config);
				string_iterate_lines(separo_extension, (void *)free);
				free(separo_extension);
				free(path_destino);
				free(path_fuente);
				free(nuevo_nombre_con_extension);
			};
		}
	}
	closedir(dp);
	t_list *bloques_temporales = list_duplicate(bloques_que_uso);
	if(!necesita_compactar)
		return;

	char *path_a_metadata = string_from_format("%sMetadata", path);
	t_config *metadata_config = config_create(path_a_metadata);
	int total_particiones = config_get_int_value(metadata_config, "PARTITIONS");
	config_destroy(metadata_config);

	for(int i=0; i < total_particiones; i++){
		char *path_a_particion = string_from_format("%s%d.bin", path, i);
		log_debug(logger, "ruta particion: %s", path_a_particion);
		t_config *particion_config = config_create(path_a_particion);
		char **bloques_usados = config_get_array_value(particion_config, "BLOCKS");
		int posicion = 0;
		while (bloques_usados[posicion] != NULL){
			list_add(bloques_que_uso, string_duplicate(bloques_usados[posicion]));
			posicion++;
		}
		string_iterate_lines(bloques_usados, (void *)free);
		free(bloques_usados);
		config_destroy(particion_config);
		free(path_a_particion);
	}
	t_list *lineas_leidas = list_create();
	int bytes_leidos = 0;
	char *parte_de_linea = NULL;
	void cargar_lineas(char *bloque){
		char *nombre_del_bloque = path_bloques();
		string_append_with_format(&nombre_del_bloque, "%s.bin", bloque);
		log_debug(logger, "[Leyendo] Bloque: %s", nombre_del_bloque);
		FILE *archivo = fopen(nombre_del_bloque, "rb");
		char *linea = malloc(sizeof(char) * maximo_caracteres_linea);

		while(fgets(linea, maximo_caracteres_linea, archivo) != NULL) {
			if(linea[strlen(linea)-1] != '\n') {
				if (parte_de_linea != NULL){
					parte_de_linea = realloc(parte_de_linea, strlen(parte_de_linea) + strlen(linea) +1);
					memset(parte_de_linea + strlen(parte_de_linea), 0, strlen(linea)+1);
					memcpy(parte_de_linea + strlen(parte_de_linea), linea, strlen(linea));
					log_warning(logger, "-%s-", parte_de_linea );
				} else {
					size_t bytes_leidos = strlen(linea);
					parte_de_linea = malloc(sizeof(char)*bytes_leidos+1);
					memset(parte_de_linea, 0, bytes_leidos+1);
					memcpy(parte_de_linea, linea, bytes_leidos);
				}
			} else {
				if(parte_de_linea != NULL) {
					char *auxiliar = malloc(strlen(linea)+1);
					memset(auxiliar, 0, strlen(linea)+1);
					memcpy(auxiliar, linea, strlen(linea));
					size_t tamanio_nuevo = strlen(linea)+strlen(parte_de_linea);
					memset(linea, 0, tamanio_nuevo);
					memcpy(linea, parte_de_linea, strlen(parte_de_linea));
					memcpy(linea+strlen(parte_de_linea), auxiliar, strlen(auxiliar));
					linea[tamanio_nuevo] = '\0';
					free(parte_de_linea);
					free(auxiliar);
					parte_de_linea = NULL;
				}
				if(linea[strlen(linea)-2] == '\0') {
					log_error(logger, "Lei barra cero");
					log_error(logger, "Lei barra cero");
					log_error(logger, "Lei barra cero");
					log_error(logger, "Lei barra cero");
				}
				//Para que no lea el barra cero
				if(strlen(linea) > 0) {
					log_debug(logger, "Leido: -%s- caracteres: %d", linea, strlen(linea));
					char **separador = string_n_split(linea, 3, ";");
					if(separador[0] != NULL || separador[1] != NULL || separador[2] != NULL) {
						log_debug(logger, "Lo agrego a leidos");
						uint16_t key_from_line = (uint16_t)strtoul(separador[1], NULL, 10);
						t_registro *registro = malloc(sizeof(t_registro));
						registro->key = key_from_line;
						registro->timestamp = atoi(separador[0]);
						registro->value = malloc(strlen(separador[2])+1);
						log_debug(logger, "Separador[2]: -%s-", separador[2]);
						memset(registro->value, 0, strlen(separador[2]+1));
						memcpy(registro->value, separador[2], strlen(separador[2]));
						registro->value[strlen(registro->value)] = '\0';
						list_add(lineas_leidas, registro);
					}
					string_iterate_lines(separador, (void*)free);
					free(separador);
					bytes_leidos += strlen(linea);
				}
			}
		}
		free(linea);
		fclose(archivo);
		free(nombre_del_bloque);
	}
	list_iterate(bloques_que_uso, (void *)cargar_lineas);
	t_list *lineas_a_compactar = list_create();
	bool _orderar_por_time_desc(t_registro *elemento, t_registro *otroElemento) {
		return elemento->timestamp > otroElemento->timestamp;
	}
	list_sort(lineas_leidas, (void*)_orderar_por_time_desc);
	limpiar_lista_de_duplicados(lineas_a_compactar, lineas_leidas);

	pthread_mutex_lock(&mutex_compactacion);
	int tiempo_inicio = get_timestamp();

	void limpiar_bitarray(char *bloque) {
		bitarray_clean_bit(datos_fs.bitarray, atoi(bloque));
	}

	void liberar_bloques_usados(char *bloque){
		free(bloque);
	}
	//list_iterate(bloques_temporales, (void *)liberar_bloques_usados);
	list_destroy_and_destroy_elements(bloques_temporales, (void *)limpiar_bitarray);
	list_destroy_and_destroy_elements(bloques_que_uso, (void *)liberar_bloques_usados);
	//list_iterate(bloques_que_uso, (void *)liberar_bloques_usados); estaría mal, si no se toca las key en la particion 1, no deberia limpiar lo que tiene
	//TODO ¿aca? list_destroy_and_destroy_elements(bloques_que_uso, (void *)liberar_bloques_usados);
	/*int bloques_necesarios = redondear_hacia_arriba(bytes_leidos, datos_fs.tamanio_bloques);
	int bloques_recibidos[bloques_necesarios];

	for(int i = 0; i < bloques_necesarios; i++){
		bloques_recibidos[i] = tomar_bloque_libre();
		if(bloques_recibidos[i] == -1){
		log_error(logger, "[COMPACTAR] ERROR: FileSystem lleno!");
			for(int base = 0; base < i; base++) {
				bitarray_clean_bit(datos_fs.bitarray, bloques_recibidos[base]);
			}
			return;
		}
	}*/
	for(int particion = 0; particion < total_particiones; particion++) {
		int cantidad_de_caracteres = 0;
		bool _filtrar_key(t_registro *linea) {
			return linea->key % total_particiones == particion;
		}
		t_list *lineas_para_particion = list_filter(lineas_a_compactar, (void *)_filtrar_key);
		if(list_size(lineas_para_particion)) {
			char *lineas_compactar = string_new();

			int caracteres_para_escribir = 0;
			void _generar_lineas(t_registro *linea) {
				//char *timestamp = string_itoa(linea->timestamp);
				//char *key = string_itoa(linea->key);
				char *linea_formada = string_from_format("%d;%d;%s", linea->timestamp, linea->key, linea->value);
				//char *linea_para_escribir = string_from_format("%s;%s;%s\n", timestamp, key, linea->value);
				string_append(&lineas_compactar, linea_formada);
				caracteres_para_escribir += strlen(linea_formada);
				free(linea_formada);
				//free(timestamp);
				//free(key);
			}
			list_iterate(lineas_para_particion, (void *)_generar_lineas);

			int cantidad_bloques = redondear_hacia_arriba(caracteres_para_escribir, datos_fs.tamanio_bloques);
			int bloques[cantidad_bloques];
			for(int i = 0; i < cantidad_bloques; i++){
				bloques[i] = tomar_bloque_libre();
				if(bloques[i] == -1){
					log_error(logger, "[CREATE] ERROR: No puedo crear el archivo tmpc, FileSystem lleno!");
					for(int base = 0; base < i; base++) {
						bitarray_clean_bit(datos_fs.bitarray, bloques[base]);
					}
					return;
				}
			}
			int bytes_a_copiar = datos_fs.tamanio_bloques;
			char *array_bloques = string_from_format("[");
			for (int j = 0; j < cantidad_bloques; j++){
				char *path = path_bloques();
				string_append_with_format(&path, "/%d.bin", bloques[j]);
				int fdopen = open(path, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
				//log_debug(logger, "bloque %d", bloques[j]);
				if(strlen(lineas_compactar) < (j*datos_fs.tamanio_bloques)){
					bytes_a_copiar = (j*datos_fs.tamanio_bloques) - strlen(lineas_compactar);
				}
				escribir(fdopen, string_substring(lineas_compactar, j*datos_fs.tamanio_bloques, bytes_a_copiar));
				close(fdopen);
				free(path);
				string_append_with_format(&array_bloques, "%d,", bloques[j]);
			}
			array_bloques[strlen(array_bloques)-1] = ']';

			char *path_a_particion = string_from_format("%s%d.bin", path, particion);
			log_debug(logger, "ruta particion: %s", path_a_particion);
			t_config *particion_config = config_create(path_a_particion);
			char **bloques_para_liberar = config_get_array_value(particion_config, "BLOCKS");
			liberar_bloques_de_particion(bloques_para_liberar);
			//string_iterate_lines(bloques_para_liberar, (void *)bloques_para_liberar);
			free(bloques_para_liberar);

			config_set_value(particion_config, "BLOCKS", array_bloques);
			config_set_value(particion_config, "SIZE", string_itoa(caracteres_para_escribir));
			config_save(particion_config);
			config_destroy(particion_config);
		}

	}
	void _borrar_temporalesc(char *ruta) {
		if(remove(ruta) == -1) perror("Error al borrar temporal de compactacion: ");
		free(ruta);
	}
	list_destroy_and_destroy_elements(archivos_a_borrar, (void *)_borrar_temporalesc);

	int tiempo_utilizado = get_timestamp() - tiempo_inicio;

	pthread_mutex_unlock(&mutex_compactacion);
	log_info(logger, "En total, la tabla %s se bloqueo %d segundos", unaTabla, tiempo_utilizado);



}


t_list *limpiar_lista_de_duplicados(t_list *lineas_a_compactar, t_list *lineas_leidas) {
	void _agregar_nuevos(t_registro *registro) {
		log_warning(logger, "Pasa por duplicados: %s", registro->value);
		bool _desde(t_registro *registro_compactar) {
			return registro_compactar->key == registro->key;

		}
		if(!list_find(lineas_a_compactar, (void *)_desde)) {
			list_add(lineas_a_compactar, registro);
		} else {
			free(registro->value);
			free(registro);
		}
	}
	list_iterate(lineas_leidas, (void *)_agregar_nuevos);
	list_destroy(lineas_leidas);
	return lineas_a_compactar;
}





void crear_archivo_tmpc(char *tabla, int size, char *datos) {
	char *ruta = string_new();
	string_append_with_format(&ruta, "%s%s/", path_tablas(), tabla);
	char *nombre_archivo = nombre_basado_en_tmpc(tabla, ruta);
	//q = (x + y - 1) / y;
	int cantidad_bloques = (size + datos_fs.tamanio_bloques - 1) / datos_fs.tamanio_bloques;
	int bloques[cantidad_bloques];
	for(int i = 0; i < cantidad_bloques; i++){
		bloques[i] = tomar_bloque_libre();
		if(bloques[i] == -1){
			log_error(logger, "[CREATE] ERROR: No puedo crear el archivo tmpc, FileSystem lleno!");
			for(int base = 0; base < i; base++) {
				bitarray_clean_bit(datos_fs.bitarray, bloques[base]);
			}
			return;
		}
	}
	int bytes_a_copiar = datos_fs.tamanio_bloques;
	for (int j = 0; j < cantidad_bloques; j++){
		char *path = path_bloques();
		string_append_with_format(&path, "/%d.bin", bloques[j]);
		int fdopen = open(path, O_RDWR | O_CREAT, S_IRWXU);
		log_debug(logger, "bloque %d", bloques[j]);
		if(strlen(datos) < (j*datos_fs.tamanio_bloques)){
			bytes_a_copiar = strlen(datos) - (j*datos_fs.tamanio_bloques);
		}
		escribir(fdopen, string_substring(datos, j*datos_fs.tamanio_bloques, bytes_a_copiar));
		close(fdopen);
		free(path);
	}
	guardar_archivo_temporal(tabla, nombre_archivo , size, bloques, cantidad_bloques);
	string_append_with_format(&ruta, "%s", nombre_archivo);
	free(nombre_archivo);

	free(ruta);
}

char *nombre_basado_en_tmpc(char *tabla, char *ruta_tabla) {
	DIR *dp;
	struct dirent *ep;
	int cantidad_de_temporales = 1;
	char *nombre = string_new();
	string_append(&nombre, tabla);
	dp = opendir(ruta_tabla);
	if (dp != NULL) {
		while ((ep = readdir (dp))) {
			if(string_contains(ep->d_name, ".tmpc"))	cantidad_de_temporales++;
		}
	}
	closedir(dp);
	string_append_with_format(&nombre, "%d.tmpc", cantidad_de_temporales);
	return nombre;
}

#include "compactar.h"

void compactar(char *unaTabla) {

	char *path = string_new();

	string_append_with_format(&path, "%s%s/", path_tablas(), unaTabla);

	DIR *dp;
	struct dirent *ep;

	t_list *bloques_que_uso = list_create();

	bool necesita_compactar = false;

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
				t_config *temporal_config = config_create(path_destino);
					char **bloques_usados = config_get_array_value(temporal_config, "BLOCKS");
					int posicion = 0;
					while (bloques_usados[posicion] != NULL){
						list_add(bloques_que_uso, bloques_usados[posicion]);
						posicion++;
					}
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
	if(!necesita_compactar)
		return;

	char *path_a_metadata = string_from_format("%sMetadata", path);
	t_config *metadata_config = config_create(path_a_metadata);
	int total_particiones = config_get_int_value(metadata_config, "PARTITIONS");
	config_destroy(metadata_config);

	for(int i=0; i < total_particiones; i++){
		char *path_a_particion = string_from_format("%s%s.bin", path, i);
		t_config *particion_config = config_create(path_a_particion);
		char **bloques_usados = config_get_array_value(metadata_config, "BLOCKS");
		int posicion = 0;
		while (bloques_usados[posicion] != NULL){
			list_add(bloques_que_uso, bloques_usados[posicion]);
			posicion++;
		}
		config_destroy(particion_config);
	}
	char *lineas_leidas = list_create();
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
							list_add(lineas_leidas, linea);
							bytes_leidos += strlen(linea);
						}
					}
					free(linea);
					fclose(archivo);
					free(nombre_del_bloque);
	}
	list_iterate(bloques_que_uso, (void *)cargar_lineas);
	t_list lineas_a_compactar = list_create();
	limpiar_lista_de_duplicados(lineas_a_compactar, lineas_leidas);

	//TODO: semaforo para bloquear operaciones de API.
	int tiempo_inicio = get_timestamp();

	void liberar_bloques_usados(char *bloque){
		bitarray_clean_bit(datos_fs.bitarray, bloque);
	}
	list_iterate(bloques_que_uso, (void *)liberar_bloques_usados);
	int bloques_necesarios = redondear_hacia_arriba(bytes_leidos, datos_fs.tamanio_bloques);
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
	}

	int tiempo_utilizado = get_timestamp() - tiempo_inicio;

	//TODO: Desbloquear semaforo
	log_info(logger, "En total, la tabla %s se bloqueo %d segundos", unaTabla, tiempo_utilizado);



}


t_list *limpiar_lista_de_duplicados(t_list *lineas_a_compactar, t_list *lineas_leidas) {
	void _agregar_nuevos(t_registro *registro) {
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
	bool _orderar_por_time_desc(t_registro *elemento, t_registro *otroElemento) {
		return elemento->timestamp > otroElemento->timestamp;
	}
	list_sort(lineas_leidas, (void*)_orderar_por_time_desc);
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
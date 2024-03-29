#include "filesystem.h"

void iniciar_fs(char *path) {
	if(path[strlen(path)-1] != '/') {
		loguear(error, logger, "La ruta del FileSystem debe terminar con /");
		exit(1);
	}
	datos_fs.path_raiz = string_duplicate(path);

	crear_sub_rutas("Tables/");
	crear_sub_rutas("Bloques/");
	crear_sub_rutas("Metadata/");
	cargar_metadata(path, "Metadata/Metadata.bin");
	cargar_metadata(path, "Metadata/Bitmap.bin");
	crear_archivos_de_bloques(datos_fs.cantidad_bloques);
}

void crear_archivos_de_bloques(int cantidad) {
	char *path_de_bloques = path_bloques();
	for(int bloque = 0; bloque < cantidad; bloque++) {
		char *un_bloque = string_from_format("%s%d.bin", path_de_bloques, bloque);
		int fd = open(un_bloque, O_RDWR | O_CREAT, S_IRWXU );
		if(fd != -1) close(fd);
		free(un_bloque);
	}
	free(path_de_bloques);
}

void cargar_metadata(char *path, char *archivo) {
	char *aux_path = string_duplicate(path);
	string_append(&aux_path, archivo);
	int fd = open(aux_path, O_RDWR, S_IRWXU );

	if(fd == -1) {
	loguear(debug, logger, "\nLevanto configuraciones %s de ejemplo", archivo);
	fd = open(aux_path, O_RDWR | O_CREAT, S_IRWXU );
		if(string_contains(archivo, "Metadata.bin")) {
			escribir(fd, "BLOCK_SIZE=64\n");
			escribir(fd, "BLOCKS=5192\n");
			escribir(fd, "MAGIC_NUMBER=LISSANDRA");
		}
		else if(string_contains(archivo, "Bitmap.bin")) {
			FILE *bitmap_file;
			unsigned char buffer_vacio[5192/8] = {'\0'};
			bitmap_file = fopen(aux_path, "wb");
			fwrite(&buffer_vacio, 5192/8, 1, bitmap_file);
			fclose(bitmap_file);
		}

	}
	if(fd != -1) {
		if(string_contains(archivo, "Metadata.bin")) {
			//printf("\n\nCARGANDO EL ARCHIVO 1\n\n");
			metadata = config_create(aux_path);
			datos_fs.tamanio_bloques = config_get_int_value(metadata, "BLOCK_SIZE");
			datos_fs.cantidad_bloques = config_get_int_value(metadata, "BLOCKS");
			config_destroy(metadata);
			//free(metadata);
		}
		else if(string_contains(archivo, "Bitmap.bin")) {
			//char *bitmap = (char*)calloc(datos_fs.cantidad_bloques, sizeof(char));
			//read(fd, bitmap, datos_fs.cantidad_bloques);
			int bytesBitmap=datos_fs.cantidad_bloques/8;
			if(datos_fs.cantidad_bloques % 8 != 0){
				bytesBitmap++;
			}
			struct stat bitmap_file_info;

			if(fstat(fd, &bitmap_file_info) == -1) perror("No pude obtener metainformacion del archivo bitmap");
			if(bitmap_file_info.st_size != bytesBitmap) {
					if(ftruncate(fd, bytesBitmap) != 0) perror("No se pudo truncar el archivo");
			}

			char *bit_mmap = mmap(NULL, bytesBitmap, PROT_READ | PROT_WRITE , MAP_SHARED, fd, 0); //write implicitamente tiene a read?
			datos_fs.bitarray = bitarray_create_with_mode(bit_mmap, bytesBitmap, MSB_FIRST);
			datos_fs._bitmap = bit_mmap;
			datos_fs._fd = fd;
			datos_fs._bytes_bitmap = bytesBitmap;
			datos_fs.ultimo_bloque_retornado = 0;
		}
	}
	if(!string_equals_ignore_case(archivo, "Bitmap.bin")) close(fd);
	free(aux_path);
}

void escribir(int fd, char *texto) {
	write(fd, texto, string_length(texto));
}

/*
 * Entrega la posicion de un bloque libre, actuando como un NEXT FIT
 * Si se pasa de la cantidad maxima, busca desde el inicio.
 * Si no tiene ningun bloque libre, retorna -1;
 */
int tomar_bloque_libre() {
	int posicion = datos_fs.ultimo_bloque_retornado;
	while(posicion < datos_fs.cantidad_bloques && bitarray_test_bit(datos_fs.bitarray, posicion)) {
		posicion++;
	}
	if(posicion >= datos_fs.cantidad_bloques) {
		posicion = 0;
		while(posicion < datos_fs.cantidad_bloques && bitarray_test_bit(datos_fs.bitarray, posicion)) {
			posicion++;
		}
		if(posicion >= datos_fs.cantidad_bloques) {
			return -1;
			//finalizar_estructuras_fs();
		}
	}
	if(posicion != SIN_BLOQUES_LIBRES) {
		bitarray_set_bit(datos_fs.bitarray, posicion);
	}
	return posicion;

}

/*
 * Quizás no sea necesaria.
 */
void crear_sub_rutas(char *archivo) {
	char *crear_en = string_duplicate(datos_fs.path_raiz);
	string_append(&crear_en, archivo);
	char **sub_rutas = string_split(crear_en, "/");
	int posicion = 1;
	loguear(info, logger, "Creando ruta: %s", crear_en);
	while(sub_rutas[posicion+1]!=NULL) {
		mkdir(crear_en, S_IRWXU);
		posicion++;
	}
	string_iterate_lines(sub_rutas, (void *)free);
	free(sub_rutas);
	free(crear_en);
}

void crear_archivo_particion(char *tabla, int particion, int bloque) {
	char *crear_en = path_tablas();
	string_append(&crear_en, tabla);
	char *raiz = string_new();
	string_append_with_format(&raiz,"%s/%d.bin", crear_en, particion);
	loguear(debug, logger, "[Particion] ruta: %s\n",raiz);
	int fd = open(raiz, O_RDWR | O_CREAT, S_IRWXU );
	escribir(fd,"SIZE=0\n");
	char *blocks = string_new();
	string_append_with_format(&blocks,"BLOCKS=[%d]", bloque);
	escribir(fd,blocks);
	close(fd);
	free(blocks);
	free(raiz);
	free(crear_en);
}

void crear_archivo_bloque(int bloque, char *contenido) {
	char *ruta = path_bloques();
	string_append_with_format(&ruta, "%d.bin", bloque);
	//string_append(&ruta, ("Bloques/%d.bin", bloque));
	//printf("\nArchivo: %s\n", ruta);

	FILE *arch_bloque;
	arch_bloque = fopen(ruta, "w");
	fwrite(contenido, strlen(contenido), 1, arch_bloque);
	fclose(arch_bloque);
	//int fd = open(ruta, O_RDWR | O_CREAT, S_IRWXU );
	//escribir(fd, contenido);
	//close(fd);
	free(ruta);
}

void crear_carpeta_tabla(char *tabla) {
	char *crear_en = path_tablas();
	string_append(&crear_en, tabla);
	mkdir(crear_en, S_IRWXU);
	free(crear_en);
}

void guardar_archivo_temporal(char *tabla, char *archivo_temporal, int size, int bloques[], int cantidad_bloques){
		char *crear_en = path_tablas();
		string_append(&crear_en, tabla);
		char *raiz = string_new();
		string_append_with_format(&raiz,"%s/%s", crear_en, archivo_temporal);
		loguear(debug, logger, "[Particion] ruta: %s\n",raiz);
		int fd = open(raiz, O_RDWR | O_CREAT, S_IRWXU );
		char *tamanio = string_new();
		string_append_with_format(&tamanio, "SIZE=%d\n", size);
		escribir(fd, tamanio );
		free(tamanio);
		char *bloques_usados = string_from_format("BLOCKS=[%d", bloques[0]);
		int index = 1;
		while (index < cantidad_bloques){
			char *bloque_string = string_from_format(", %d", bloques[index]);
			string_append(&bloques_usados, bloque_string);
			free(bloque_string);
			index ++;
		}
		string_append(&bloques_usados, "]");
		escribir(fd,bloques_usados);
		close(fd);
		free(bloques_usados);
		free(raiz);
		free(crear_en);
}


void guardar_archivo_metadata(char *tabla, char *criterio, int particiones, int compaction_time) {
	char *crear_en = path_tablas();
	string_append(&crear_en, tabla);
	string_append(&crear_en, "/Metadata");
	char *consistency = string_new();
	string_append_with_format(&consistency,"CONSISTENCY=%s\n",criterio);
	char *partition = string_new();
	string_append_with_format(&partition,"PARTITIONS=%d\n",particiones);
	char *compaction = string_new();
	string_append_with_format(&compaction,"COMPACTION_TIME=%d",compaction_time);
	int fd = open(crear_en, O_RDWR | O_CREAT, S_IRWXU );
	escribir(fd, consistency);
	escribir(fd, partition);
	escribir(fd, compaction);
	close(fd);
	free(consistency);
	free(partition);
	free(compaction);
	free(crear_en);
}

/*void guardar_bitmap(char *path_bitmap) {
	int fd = open(path_bitmap, O_RDWR | O_CREAT, S_IRWXU );
	//char *bitmap = (char*)calloc(datos_fs.cantidad_bloques, sizeof(char));
	char *bitmap = string_duplicate(datos_fs.bitarray->bitarray);
	//printf("%s\n", bitmap);
	escribir(fd, bitmap);
	close(fd);
	free(bitmap);
	//printf("\nguardado en %s\n", path_bitmap);
}*/

char *path_tablas() {
	char *ruta_archivo = string_duplicate(datos_fs.path_raiz);
	string_append(&ruta_archivo, "Tables/");
	return ruta_archivo;
}

char *path_bloques() {
	char *ruta_archivo = string_duplicate(datos_fs.path_raiz);
	string_append(&ruta_archivo, "Bloques/");
	return ruta_archivo;
}

void finalizar_estructuras_fs() {
	loguear(info, logger, "Saliendo...");
	/*char *ruta_bitmap = datos_fs.path_raiz;
	string_append(&ruta_bitmap, "Metadata/Bitmap.bin");
	guardar_bitmap(ruta_bitmap);
	free(ruta_bitmap);*/

	bitarray_destroy(datos_fs.bitarray);
	if(munmap(datos_fs._bitmap, datos_fs._bytes_bitmap) == 1) loguear(error, logger, "Error en munmap()");
	close(datos_fs._fd);
	free(datos_fs.path_raiz);
}

bool existe_tabla(char *tabla) {
	loguear(info, logger, "[Busqueda] Verifico si existe tabla");
	char *buscar_en = path_tablas();
	bool resultado = false;
	struct dirent *de;

	DIR *dr = opendir(buscar_en);

	if (dr == NULL) //Si falla la apertura mato el proceso, porque no puedo determinar si existe o no
	{
		perror("error al abrir directorio");
		exit(1);
	}

	//Opcional: ignorar . y ..
	while ((de = readdir(dr)) != NULL) {
		//Si la commons nos brinda case insensitive no veo el objetivo de comparar en mayúsculas o minúsculas
		if(string_equals_ignore_case(de->d_name, tabla)) {
			resultado = true;
		}
	}

	closedir(dr);

	free(buscar_en);
	loguear(info, logger, resultado ? "[Busqueda - Resultado] Se encontro la tabla" : "[Busqueda - Resultado] No se encontro la tabla");
	return resultado;
}


t_list *obtener_datos_de_particion(char *path, uint16_t key) {
		t_list *retorno = list_create();
		char *buscar_en = path_tablas();
		string_append(&buscar_en, path);
		FILE *archivo;

		loguear(debug, logger, "Busco en: %s", buscar_en);
		datos_archivo = config_create(buscar_en);
		int max_tamanio = config_get_int_value(datos_archivo, "SIZE");
		char** bloques_usados = config_get_array_value(datos_archivo, "BLOCKS");
		config_destroy(datos_archivo);
		free(buscar_en);

		int total_de_bloques_usados_por_archivo = 0;

		while(bloques_usados[total_de_bloques_usados_por_archivo] != NULL) {
			total_de_bloques_usados_por_archivo++;
		}

		char *linea;
		char *parte_de_linea = NULL;

		bool key_encontrada = false;
		for(int indice_bloque = 0; indice_bloque < total_de_bloques_usados_por_archivo; indice_bloque++) {
			char *nombre_del_bloque = path_bloques();
			string_append_with_format(&nombre_del_bloque, "%s.bin", bloques_usados[indice_bloque]);
			loguear(debug, logger, "[Leyendo] Bloque: %s", nombre_del_bloque);
			archivo = fopen(nombre_del_bloque, "rb");
			linea = malloc(sizeof(char) * maximo_caracteres_linea);

			while(fgets(linea, maximo_caracteres_linea, archivo) != NULL) {
				if(linea[strlen(linea)-1] != '\n') {
					if (parte_de_linea != NULL){
						parte_de_linea = realloc(parte_de_linea, strlen(parte_de_linea) + strlen(linea) +1);
						memset(parte_de_linea + strlen(parte_de_linea), 0, strlen(linea)+1);
						memcpy(parte_de_linea + strlen(parte_de_linea), linea, strlen(linea));
						//parte_de_linea[strlen(parte_de_linea)] = '\0';
						loguear(warning, logger, "-%s-", parte_de_linea );
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
					if(matchea_key_en_linea(linea, key)) {
						loguear(info, logger, "[Key encontrada] %s", linea);
						//retorno = cargar_datos_timestamp_value(linea);
						list_add(retorno, string_duplicate(linea));
						//key_encontrada = true;
						//break;
					}
				}
			}
			free(linea);
			fclose(archivo);
			free(nombre_del_bloque);

		}

		string_iterate_lines(bloques_usados, (void*) free);
		free(bloques_usados);
		return retorno;
}

t_timestamp_value *cargar_datos_timestamp_value(char *linea) {
	char **separador = string_n_split(linea, 3, ";");
	t_timestamp_value *retorno = malloc(sizeof(t_timestamp_value));
	retorno->timestamp = string_to_timestamp(separador[0]);
	retorno->value = string_duplicate(separador[2]);
	string_iterate_lines(separador, (void*)free);
	free(separador);
	return retorno;
}

bool matchea_key_en_linea(char *linea, uint16_t key) {
	char **separador = string_n_split(linea, 3, ";");
	if(separador[0] == NULL || separador[1] == NULL) {
		string_iterate_lines(separador, (void*)free);
		free(separador);
		return false;
	}
	uint16_t key_from_file = (uint16_t)strtoul(separador[1], NULL, 10);
	string_iterate_lines(separador, (void*)free);
	free(separador);
	return key == key_from_file;
}

t_metadata obtener_metadata(char *tabla) {
	t_metadata retorno;
	char *ruta = path_tablas();
	string_append_with_format(&ruta,"%s/Metadata",tabla);
	t_config *conf = config_create(ruta);
	retorno.consistency = strdup(config_get_string_value(conf,"CONSISTENCY"));
	retorno.partitions = (uint16_t)config_get_int_value(conf,"PARTITIONS");
	retorno.compaction_time = config_get_int_value(conf,"COMPACTION_TIME");
	config_destroy(conf);
	free (ruta);
	return retorno;
}

t_memtable *obtener_tabla_en_memtable(char *tabla) {
	bool _buscar_por_nombre(t_memtable *elemento) {
		return string_equals_ignore_case(elemento->tabla, tabla);
	}
	return (t_memtable*)list_find(t_list_memtable, (void *)_buscar_por_nombre);
}

t_list *obtener_registros_por_key(char *tabla, uint16_t key) {
	t_list *retorno = list_create();

	t_memtable *area_de_tabla = obtener_tabla_en_memtable(tabla);
	/*
	 * Preguntar si esto va, en caso de key duplicada en el memtable
	 * orderno por las que tengan el timestamp mas grande primero
	 * entonces va a devolver la mas actualizada
	 * Definir si esta operacion va en el insert, o dejarla aca.
	 * Si se hace en el insert, nos olvidamos de ordenar cuando
	 * se realiza el dump
	 */
	if(!area_de_tabla) return retorno;
	/*bool _orderar_por_time_desc(t_registro *elemento, t_registro *otroElemento) {
		return elemento->timestamp > otroElemento->timestamp;
	}
	list_sort(area_de_tabla->t_registro, (void*)_orderar_por_time_desc);*/
	void _buscar_por_key(t_registro *elemento) {
		if (elemento->key == key){
			char *elemento_string = string_from_format("%llu;%d;%s", elemento->timestamp, elemento->key, elemento->value);
			list_add(retorno, elemento_string);
		}
	}
	list_iterate(area_de_tabla->t_registro, (void*)_buscar_por_key);
	return retorno;
}

void printear_memtable() {
	loguear(debug, logger, "Tablas en la memtable::");
	void _mostrar_nombre(void *unaTabla) {
		void _mostrar_valores(void *unRegistro) {
			loguear(debug, logger, "%llu - %d - %s", ((t_registro*)unRegistro)->timestamp , ((t_registro*)unRegistro)->key, ((t_registro*)unRegistro)->value);
		}
		//loguear(debug(logger, "Tabla - %s", (*(t_memtable*)unaTabla).tabla);
		loguear(warning, logger, "Tabla - %s", ((t_memtable*)unaTabla)->tabla);
		list_iterate(((t_memtable*)unaTabla)->t_registro, _mostrar_valores);
		printf("______________________________________________\n");
	}
	list_iterate(t_list_memtable, _mostrar_nombre);
	printf("\n\n");
}

void limpiar_registros_memtable(t_registro *unRegistro) {
	free(unRegistro->value);
	free(unRegistro);
}

void limpiar_tablas_memtable(t_memtable *unaTabla) {
	free(unaTabla->tabla);
	list_destroy_and_destroy_elements(unaTabla->t_registro, (void*)limpiar_registros_memtable);
	free(unaTabla);
}

t_timestamp_value *devolver_timestamp_mayor(t_list *lista) {

	t_timestamp_value *aux = NULL;
	void _buscar_mayor(char* unaLinea){
		if (!unaLinea) return;
		char **separador = string_n_split(unaLinea, 3, ";");
		if (!aux) {
			aux = malloc(sizeof(t_timestamp_value));
			aux->timestamp = string_to_timestamp(separador[0]);
			aux->value = string_duplicate(separador[2]);
		}
		else if (aux->timestamp <= string_to_timestamp(separador[0])) {
			free(aux->value);
			aux->timestamp = string_to_timestamp(separador[0]);
			aux->value = string_duplicate(separador[2]);

		}
		string_iterate_lines(separador, (void*)free);
		free(separador);
	}
	list_iterate(lista,(void*)_buscar_mayor);

	return aux;
	/*t_timestamp_value *aux;
	if(!uno && !otro) {
		return NULL;
	} if(!uno && otro) {
		return otro;
	} else if(uno && !otro) {
		return uno;
	} else {
		uno->timestamp >= otro->timestamp ? (aux = uno) : (aux = otro);
	}
	return aux;*/
}

void limpiar_timestampvalue_si_corresponde(t_timestamp_value *registro) {
	if(!registro) return;
	free(registro->value);
	free(registro);
}

t_response_describe *devolver_metadata(char *path_tabla, char *tabla) {
	char *path_a_metada = string_new();
	string_append_with_format(&path_a_metada, "%s/%s", path_tabla, "Metadata");
	t_config *metadata_config = config_create(path_a_metada);
	char *consistenca_string = strdup(config_get_string_value(metadata_config, "CONSISTENCY"));
	criterio consistencia = criterio_from_string(consistenca_string);
	free(consistenca_string);
	config_destroy(metadata_config);
	free(path_a_metada);
	t_response_describe *retorno = malloc(sizeof(t_response_describe));
	retorno->tabla = string_duplicate(tabla);
	retorno->consistencia = consistencia;
	return retorno;
}

void liberar_bloques_de_particion(char **lista_bloques) {}



bool removerArchivo(char* tabla){
	char *buscar_en = path_tablas();
	string_append(&buscar_en, tabla);

	bool resultado = false;
	struct dirent *de;

	DIR *dr = opendir(buscar_en);

	if (dr == NULL) //Si falla la apertura mato el proceso, porque no puedo determinar si existe o no
	{
		perror("error al abrir directorio");
		return false;
	}
	//Opcional: ignorar . y ..
	while ((de = readdir(dr)) != NULL) {
		char *ruta_completa = string_duplicate(buscar_en);
		string_append_with_format(&ruta_completa, "/%s", de->d_name);
		if(string_contains(de->d_name, ".bin") || string_contains(de->d_name, ".tmp") ) {

			loguear(info, logger, "%s", ruta_completa);
			t_config *particion_config = config_create(ruta_completa);
					char **bloques_usados = config_get_array_value(particion_config, "BLOCKS");
					int posicion = 0;
					while (bloques_usados[posicion] != NULL){
						bitarray_clean_bit(datos_fs.bitarray, atoi(bloques_usados[posicion]));
						posicion++;
					}
					string_iterate_lines(bloques_usados, (void *)free);
					free(bloques_usados);
					config_destroy(particion_config);
					remove(ruta_completa);
		}else if(!string_starts_with(de->d_name, ".")){
			remove(ruta_completa);
		}
		free(ruta_completa);
	}
	closedir(dr);
	rmdir(buscar_en);
	free(buscar_en);
	return true;
}

void exportar() {
	char *buscar_en = path_tablas();
	struct dirent *de;

	DIR *dr = opendir(buscar_en);

	char *guardar_en = string_duplicate(datos_fs.path_raiz);
	string_append(&guardar_en, "exportacion.xls");
	int fd = open(guardar_en, O_RDWR | O_CREAT, S_IRWXU );
	if (dr == NULL) //Si falla la apertura mato el proceso, porque no puedo determinar si existe o no
	{
		perror("error al abrir directorio");
		exit(1);
	}


	//Opcional: ignorar . y ..
	while ((de = readdir(dr)) != NULL) {
		t_list *bloques_que_uso = list_create();
		//Si la commons nos brinda case insensitive no veo el objetivo de comparar en mayúsculas o minúsculas
		if(!string_contains(de->d_name, ".")) {
			char *tabla_descripcion = string_from_format("TABLA %s;\n", de->d_name);
			escribir(fd, tabla_descripcion);
			free(tabla_descripcion);

			char *path = string_from_format("%s%s/", buscar_en, de->d_name);




			char *path_a_metadata = string_from_format("%sMetadata", path);
			t_config *metadata_config = config_create(path_a_metadata);
			int total_particiones = config_get_int_value(metadata_config, "PARTITIONS");
			config_destroy(metadata_config);
			free(path_a_metadata);

			for(int i=0; i < total_particiones; i++){
				char *path_a_particion = string_from_format("%s%d.bin", path, i);
				loguear(debug, logger, "ruta particion: %s", path_a_particion);
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
			//t_list *lineas_leidas = list_create();
			int bytes_leidos = 0;
			char *parte_de_linea = NULL;
			void cargar_lineas(char *bloque){
				char *nombre_del_bloque = path_bloques();
				string_append_with_format(&nombre_del_bloque, "%s.bin", bloque);
				loguear(debug, logger, "[Leyendo] Bloque: %s", nombre_del_bloque);
				FILE *archivo = fopen(nombre_del_bloque, "rb");
				char *linea = malloc(sizeof(char) * maximo_caracteres_linea);

				while(fgets(linea, maximo_caracteres_linea, archivo) != NULL) {
					if(linea[strlen(linea)-1] != '\n') {
						if (parte_de_linea != NULL){
							parte_de_linea = realloc(parte_de_linea, strlen(parte_de_linea) + strlen(linea) +1);
							memset(parte_de_linea + strlen(parte_de_linea), 0, strlen(linea)+1);
							memcpy(parte_de_linea + strlen(parte_de_linea), linea, strlen(linea));
							loguear(warning, logger, "-%s-", parte_de_linea );
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
							loguear(error, logger, "Lei barra cero");
							loguear(error, logger, "Lei barra cero");
							loguear(error, logger, "Lei barra cero");
							loguear(error, logger, "Lei barra cero");
						}
						//Para que no lea el barra cero
						if(strlen(linea) > 0) {
							loguear(debug, logger, "Leido: -%s- caracteres: %d", linea, strlen(linea));
							char **separador = string_n_split(linea, 3, ";");
							if(separador[0] != NULL || separador[1] != NULL || separador[2] != NULL) {
								loguear(debug, logger, "Lo agrego a leidos");
								uint16_t key_from_line = (uint16_t)strtoul(separador[1], NULL, 10);
								char *string = string_from_format("%d;%s", key_from_line, separador[2]);
								escribir(fd, string);
								free(string);
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









			free(path);

		}
		list_destroy_and_destroy_elements(bloques_que_uso, (void *)free);
	}

	closedir(dr);
	close(fd);
	free(guardar_en);

	free(buscar_en);
}

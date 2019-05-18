#include "filesystem.h"

void iniciar_fs(char *path) {
	datos_fs.path_raiz = string_duplicate(path);
	cargar_metadata(path, "Metadata/Metadata.bin");
	cargar_metadata(path, "Metadata/Bitmap.bin");
	/*printf("\nTOTAL BLOQUES EN BYTES DESDE BITMAP: %d\n", datos_fs.bitarray->size);
	bitarray_set_bit(datos_fs.bitarray, 16);
	//bitarray_set_bit(datos_fs.bitarray, 5191*8);
	printf("\nBITMAP: %s\n", datos_fs.bitarray->bitarray);*/
	//bitarray_set_bit(datos_fs.bitarray, 0);

}

void cargar_metadata(char *path, char *archivo) {
	char *aux_path = string_duplicate(path);
	string_append(&aux_path, archivo);
	int fd = open(aux_path, O_RDWR, S_IRWXU );

	if(fd == -1) {
	log_debug(logger, "\nLevanto configuraciones %s de ejemplo", archivo);
	fd = open(aux_path, O_RDWR | O_CREAT, S_IRWXU );
		if(string_contains(archivo, "Metadata.bin")) {
			escribir(fd, "BLOCK_SIZE=64\n");
			escribir(fd, "BLOCKS=5192\n");
			escribir(fd, "MAGIC_NUMBER=LISSANDRA\n");
		}
		else if(string_contains(archivo, "Bitmap.bin")) {
			char *bitmap = (char*)calloc(64, sizeof(char));
			bitmap = string_repeat('0', 64);
			escribir(fd, bitmap);
			free(bitmap);
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
			char *bitmap = (char*)calloc(datos_fs.cantidad_bloques, sizeof(char));
			read(fd, bitmap, datos_fs.cantidad_bloques);
			int bytesBitmap=datos_fs.cantidad_bloques/8;
			if(datos_fs.cantidad_bloques % 8 != 0){
				bytesBitmap++;
			}
			datos_fs.bitarray = bitarray_create_with_mode(bitmap,bytesBitmap,MSB_FIRST);
			datos_fs.ultimo_bloque_retornado = 0;
		}
	}
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
bool crear_sub_rutas(char *archivo) {
	char *crear_en = string_duplicate(path_tablas());
	char **sub_rutas = string_split(archivo, "/");
	int posicion = 0;
	while(sub_rutas[posicion+1]!=NULL) {
		log_info(logger, "Creando ruta: %s", sub_rutas[posicion]);
		string_append(&crear_en, string_from_format("%s/",sub_rutas[posicion++]));
		mkdir(crear_en, S_IRWXU);
	}

	free(crear_en);
	return true;
}

void crear_archivo_bloque(int bloque, char *contenido) {
	char *ruta = string_duplicate(datos_fs.path_raiz);
	string_append(&ruta, string_from_format("Bloques/%d.bin", bloque));
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

void guardar_archivo_metadata(char *tabla, char *criterio, int particiones, int compaction_time) {
	char *crear_en = string_duplicate(path_tablas());
	char *raiz=malloc(strlen(crear_en));
	for (int i=0; i<particiones; i++){
		int largo_raiz = strlen(raiz)==0?strlen(crear_en):strlen(raiz);
		memset(raiz,0,largo_raiz);
		string_append_with_format(&raiz,"%s%s/%d.bin",crear_en,tabla,i);
		printf("%s\n",raiz);
		int fd = open(raiz, O_RDWR | O_CREAT, S_IRWXU );
		escribir(fd,"");
		close(fd);
	}

	free(raiz);

	string_append(&crear_en, tabla);
	string_append(&crear_en, "/Metadata");
	char *consistency = malloc(sizeof (char)*15);
	string_append_with_format(&consistency,"CONSISTENCY=%s\n",criterio);
	char *partition = malloc(sizeof (char)*15);
	string_append_with_format(&partition,"PARTITIONS=%d\n",particiones);
	char *compaction = malloc(sizeof (char)*15);
	string_append_with_format(&compaction,"COMPACTION_TIME=%d\n",compaction_time);
	int fd = open(crear_en, O_RDWR | O_CREAT, S_IRWXU );
	escribir(fd, consistency);
	escribir(fd, partition);
	escribir(fd, compaction);
	free(consistency);
	free(partition);
	free(compaction);
	close(fd);
}

void guardar_bitmap(char *path_bitmap) {
	int fd = open(path_bitmap, O_RDWR | O_CREAT, S_IRWXU );
	//char *bitmap = (char*)calloc(datos_fs.cantidad_bloques, sizeof(char));
	char *bitmap = string_duplicate(datos_fs.bitarray->bitarray);
	//printf("%s\n", bitmap);
	escribir(fd, bitmap);
	close(fd);
	free(bitmap);
	//printf("\nguardado en %s\n", path_bitmap);
}

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

/* NUNCA LA LLAMO, PERO LA DEJO PREPARADA */
void finalizar_estructuras_fs() {
	log_info(logger, "Saliendo...");
	char *ruta_bitmap = string_duplicate(datos_fs.path_raiz);
	string_append(&ruta_bitmap, "Metadata/Bitmap.bin");
	guardar_bitmap(ruta_bitmap);
	free(ruta_bitmap);
	free(path_tablas());
	bitarray_destroy(datos_fs.bitarray);
}

bool existe_tabla(char *tabla) {
	log_info(logger, "[Busqueda] Verifico si existe tabla");
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
	log_info(logger, resultado ? "[Busqueda - Resultado] Se encontro la tabla" : "[Busqueda - Resultado] No se encontro la tabla");
	return resultado;
}


char* obtener_datos(char *path, uint16_t key) {
	//if(validar_archivo(path)) {
		char *buscar_en = string_duplicate(path_tablas());
		string_append(&buscar_en, path);
		FILE *archivo;
		//char *buffer;
		//long filelen;

		datos_archivo = config_create(buscar_en);
		int max_tamanio = config_get_int_value(datos_archivo, "TAMANIO");
		char** bloques_usados = config_get_array_value(datos_archivo, "BLOQUES");
		config_destroy(datos_archivo);
		/*if((offset + size) > max_tamanio) {
			printf("\nError al obtener datos: se pasa del tamanio del archivo\n");
			exit(1);
		}*/

		int total_de_bloques_usados_por_archivo = 0;

		while(bloques_usados[total_de_bloques_usados_por_archivo] != NULL) {
			total_de_bloques_usados_por_archivo++;
		}
		printf("Total de bloques: %d", total_de_bloques_usados_por_archivo);
		/*
		int *bloques = (int*)calloc(aux_bloques_usados, sizeof(int));
		for(int i = 0; i < aux_bloques_usados;i++) {
			bloques[i] = atoi(strdup(bloques_usados[i]));
			//printf("\nlectura bloque: %d\n", bloques[i]);
		}*/
		int maximo_caracteres_linea = 12 + 2 + 5+ fs_config.tamanio_value; //una linea se forma de maximo int (12 caracteres) 2 ; , un uint16, y el value
		char *buffer;
		bool key_encontrada = false;
		for(int indice_bloque = 0; indice_bloque < total_de_bloques_usados_por_archivo; indice_bloque++) {
			char *nombre_del_bloque = string_new();
			nombre_del_bloque = string_duplicate(path_bloques());
			string_append_with_format(&nombre_del_bloque, "%s.bin", bloques_usados[indice_bloque]);
			printf("Estoy en el archivo: %s\n", nombre_del_bloque);
			archivo = fopen(nombre_del_bloque, "rb");
			buffer = malloc(sizeof(char) * maximo_caracteres_linea);

			/*fread(buffer, size, 1, archivo);
			buffer[size] = '\0';
			printf("\nBuffer: %s\n", buffer);*/
			while(fgets(buffer, maximo_caracteres_linea, archivo) != NULL) {
				char **separador = string_n_split(buffer, 3, ";");
				uint16_t key_from_file = (uint16_t)strtoul(separador[1], NULL, 10);
				if(key == key_from_file) {
					printf("Encontre la key!!!!!\n");
					key_encontrada = true;
					break;
				}
				/*
				strtoul al key con el que recibo por arguemnto
				si lo encuentra corto el while, salgo del for.
				retorno el value o la linea .*/
				//printf("Value en esta linea: %s", separador[2]);
			}
			if(key_encontrada) {
				break;
			} else {
				free(buffer);
			}
			fclose(archivo);

		}

		//free(nombre_del_bloque);
		//free(bloques);
		string_iterate_lines(bloques_usados, (void*) free);
		free(bloques_usados);
		free(buscar_en);
		return buffer;
	//}
}

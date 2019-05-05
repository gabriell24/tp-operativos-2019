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

//Todo pasarle los parámetros al write
void guardar_archivo_metadata(char *tabla, char *criterio, int particiones, int compaction_time) {
	char *crear_en = string_duplicate(path_tablas());
	string_append(&crear_en, tabla);
	string_append(&crear_en, "/Metadata");
	int fd = open(crear_en, O_RDWR | O_CREAT, S_IRWXU );
	escribir(fd, "CONSISTENCY=SC\n");
	escribir(fd, "PARTITIONS=4\n");
	escribir(fd, "COMPACTION_TIME=60000");
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

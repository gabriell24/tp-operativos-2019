#include "dump.h"

void dump_automatico() {
	while(!consola_ejecuto_exit)
	{
		usleep(fs_config.tiempo_dump_ms * 1000);
		//log_info(logger, "[Dump] Ejecutando dump");
		//dumpear();
	}
}
void dumpear() {
	char *linea = string_new();

	void _obtener_tablas(t_memtable *unaTabla) {
//		FILE *archivo = crear_y_devolver_archivo_temporal(unaTabla->tabla);
//		if(archivo == NULL) {
//			log_error(logger, "[Dump] Error, no se pudo crear archivo para dumpear");
//		}
		void _writear_valores(t_registro *unRegistro) {
			//char *linea = string_new();
			string_append_with_format(&linea, "%d;%d;%s\n", unRegistro->timestamp , unRegistro->key, unRegistro->value);
			//fwrite(linea, strlen(linea), 1, archivo);
		}
		bool _orderar_por_time_desc(t_registro *elemento, t_registro *otroElemento) {
			return elemento->timestamp > otroElemento->timestamp;
		}
		list_sort(unaTabla->t_registro, (void*)_orderar_por_time_desc);
		list_iterate(unaTabla->t_registro, (void *)_writear_valores);
		crear_archivo_temporal(unaTabla->tabla, strlen(linea), linea);
		log_info(logger, linea);
		//fclose(archivo);
	}
	list_iterate(t_list_memtable, (void *)_obtener_tablas);
	list_clean_and_destroy_elements(t_list_memtable, (void*)limpiar_tablas_memtable);
	t_list_memtable = list_create();
}

//falta añadir el size (bytes) y los bloquess que ocupa
void crear_archivo_temporal(char *tabla, int size, char *datos) {
	char *ruta = string_new();
	string_append_with_format(&ruta, "%s%s/", path_tablas(), tabla);
	char *nombre_archivo = nombre_basado_en_temporales(tabla, ruta);
	//q = (x + y - 1) / y;
	int cantidad_bloques = (size + datos_fs.tamanio_bloques - 1) / datos_fs.tamanio_bloques;
	int bloques[cantidad_bloques];
	for(int i = 0; i < cantidad_bloques; i++){
		bloques[i] = tomar_bloque_libre();
		if(bloques[i] == -1){
			log_error(logger, "[CREATE] ERROR: No puedo crear temporal, FileSystem lleno!");
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

char *nombre_basado_en_temporales(char *tabla, char *ruta_tabla) {
	DIR *dp;
	struct dirent *ep;
	int cantidad_de_temporales = 1;
	char *nombre = string_new();
	string_append(&nombre, tabla);
	dp = opendir(ruta_tabla);
	if (dp != NULL) {
		while ((ep = readdir (dp))) {
			if(string_contains(ep->d_name, ".tmp"))	cantidad_de_temporales++;
		}
	}
	closedir(dp);
	string_append_with_format(&nombre, "%d.tmp", cantidad_de_temporales);
	return nombre;
}

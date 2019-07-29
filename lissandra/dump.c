#include "dump.h"

void dump_automatico() {
	while(!finalizo_proceso())
	{
		loguear(info, logger, "[Dump] Ejecutando dump");
		dumpear();
		loguear(info, logger, "[Dump] Finalizado.");
		usleep(fs_config.tiempo_dump_ms * 1000);
	}
}

void dumpear() {

	pthread_mutex_lock(&mutex_memtable);
	void _obtener_tablas(t_memtable *unaTabla) {
		char *linea = string_new();
		void _writear_valores(t_registro *unRegistro) {
			string_append_with_format(&linea, "%llu;%d;%s\n", unRegistro->timestamp , unRegistro->key, unRegistro->value);
		}
		bool _orderar_por_time_desc(t_registro *elemento, t_registro *otroElemento) {
			return elemento->timestamp > otroElemento->timestamp;
		}
		list_sort(unaTabla->t_registro, (void*)_orderar_por_time_desc);
		list_iterate(unaTabla->t_registro, (void *)_writear_valores);
		crear_archivo_temporal(unaTabla->tabla, strlen(linea), linea);
		free(linea);
	}
	list_iterate(t_list_memtable, (void *)_obtener_tablas);
	//list_destroy_and_destroy_elements(t_list_memtable, (void*)limpiar_tablas_memtable);
	list_clean_and_destroy_elements(t_list_memtable, (void*)limpiar_tablas_memtable);
	pthread_mutex_unlock(&mutex_memtable);
	//t_list_memtable = list_create();
}

//falta añadir el size (bytes) y los bloquess que ocupa
void crear_archivo_temporal(char *tabla, int size, char *datos) {
	char *ruta = string_new();
	char *path_tabla = path_tablas();
	string_append_with_format(&ruta, "%s%s/", path_tabla, tabla);
	free(path_tabla);
	char *nombre_archivo = nombre_basado_en_temporales(tabla, ruta);
	//q = (x + y - 1) / y;
	int cantidad_bloques = (size + datos_fs.tamanio_bloques - 1) / datos_fs.tamanio_bloques;
	int bloques[cantidad_bloques];
	for(int i = 0; i < cantidad_bloques; i++){
		bloques[i] = tomar_bloque_libre();
		if(bloques[i] == -1){
			loguear(error, logger, "[CREATE] ERROR: No puedo crear temporal, FileSystem lleno!");
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
		int fdopen = open(path, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
		loguear(debug, logger, "bloque %d", bloques[j]);
		if(strlen(datos) < (j*datos_fs.tamanio_bloques)){
			bytes_a_copiar = (j*datos_fs.tamanio_bloques) - strlen(datos);
		}
		char *a_escribir = string_substring(datos, j*datos_fs.tamanio_bloques, bytes_a_copiar);
		escribir(fdopen, a_escribir);
		free(a_escribir);
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
	//int cantidad_de_temporales = 1;
	int maximo_temporal = 0;
	char *nombre = string_new();
	string_append(&nombre, tabla);
	dp = opendir(ruta_tabla);
	if (dp != NULL) {
		while ((ep = readdir (dp))) {
			if(string_ends_with(ep->d_name, ".tmp")){
				int numero_temporal = number_string(ep->d_name);
				if(numero_temporal > maximo_temporal) maximo_temporal = numero_temporal;
			}
		}
	}
	closedir(dp);
	string_append_with_format(&nombre, "%d.tmp", maximo_temporal +1);
	return nombre;
}

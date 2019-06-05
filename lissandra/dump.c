#include "dump.h"

void dump_automatico() {
	while(!consola_ejecuto_exit)
	{
		usleep(fs_config.tiempo_dump_ms * 1000);
		log_info(logger, "[Dump] Ejecutando dump");
		dumpear();
	}
}
void dumpear() {
	void _obtener_tablas(t_memtable *unaTabla) {
		FILE *archivo = crear_y_devolver_archivo_temporal(unaTabla->tabla);
		if(archivo == NULL) {
			log_error(logger, "[Dump] Error, no se pudo crear archivo para dumpear");
		}
		void _writear_valores(t_registro *unRegistro) {
			char *linea = string_new();
			string_append_with_format(&linea, "%d;%d;%s\n", unRegistro->timestamp , unRegistro->key, unRegistro->value);
			fwrite(linea, strlen(linea), 1, archivo);
		}
		bool _orderar_por_time_desc(t_registro *elemento, t_registro *otroElemento) {
			return elemento->timestamp > otroElemento->timestamp;
		}
		list_sort(unaTabla->t_registro, (void*)_orderar_por_time_desc);
		list_iterate(unaTabla->t_registro, (void *)_writear_valores);
		fclose(archivo);
	}
	list_iterate(t_list_memtable, (void *)_obtener_tablas);
	list_clean_and_destroy_elements(t_list_memtable, (void*)limpiar_tablas_memtable);
	t_list_memtable = list_create();
}

FILE *crear_y_devolver_archivo_temporal(char *tabla) {
	char *ruta = string_new();
	string_append_with_format(&ruta, "%s%s/", path_tablas(), tabla);
	char *nombre_archivo = nombre_basado_en_temporales(tabla, ruta);
	string_append_with_format(&ruta, "%s", nombre_archivo);
	free(nombre_archivo);
	FILE *archivo;
	/*
	 * OJO QUE LO PONGO COMO UN WRITE CON APPREND, QUIZAS SEA SOLO WRITE O TRUNCATE
	 */
	log_debug(logger, "El directorio final del archivo sera: %s", ruta);
	archivo = fopen(ruta, "w+");

	free(ruta);
	return archivo;
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

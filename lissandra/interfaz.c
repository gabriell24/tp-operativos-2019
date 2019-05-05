#include "interfaz.h"

void fs_select(char *tabla, char *clave) {
	//Dummy
}

void fs_insert(char *tabla, char *clave, char *value, int timestamp) {
	log_info(logger, "[EPOCH] timestamp: %d", timestamp);
}

void fs_create(char *tabla, char *tipo_consistencia, int particiones, int tiempo_compactacion) {
	if(existe_tabla(tabla)) {
		log_error(logger, "[CREATE] ERROR: Ya existe tabla con ese nombre.");
		return;
	}
	crear_carpeta_tabla(tabla);
	//todo arreglar la función
	guardar_archivo_metadata(tabla, tipo_consistencia, particiones, tiempo_compactacion);
}

void fs_describe(char *tabla) {
	bool mostrar_todo = tabla == NULL;
	//Dummy
}

void fs_drop(char *tabla) {
	//Dummy
}


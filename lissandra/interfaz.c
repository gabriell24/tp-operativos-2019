#include "interfaz.h"

void fs_select(char *tabla, char *clave) {
	//Dummy
}

void fs_insert(char *tabla, char *clave, char *value, int timestamp) {
	log_info(logger, "[EPOCH] timestamp: %d", timestamp);
}

void fs_create(char *tabla, char *tipo_consistencia, int particiones, int tiempo_compactacion) {
	//Dummy
}

void fs_describe(char *tabla) {
	bool mostrar_todo = tabla == NULL;
	//Dummy
}

void fs_drop(char *tabla) {
	//Dummy
}


#include "interfaz.h"

void kernel_select(char *tabla, char *clave) {
	printf("Holis soy Rodrigos");
	//Dummy
	size_t tamanio_del_buffer = sizeof(int)*3 + strlen(tabla) + strlen(clave);
	void *buffer = serializar_request_select(tabla, clave);
	prot_enviar_mensaje(socket_memoria, FUNCION_SELECT, tamanio_del_buffer, buffer);
	log_info(logger, "Select enviado a memoria");
}

void kernel_insert(char* nombre_tabla, char* key, char* value, int epoch){

	printf("Hola mundo :) hiciste un insert a kernel (?");
}

void kernel_create (char* nombre_tabla, char* tipo_consistencia, int numero_particiones, int compactation_time){
	printf("Holis :) hiciste un create desde kernel");
}


void imprimir_metricas() {
	printf("Implementando métricas version: %d\n", 1);

}

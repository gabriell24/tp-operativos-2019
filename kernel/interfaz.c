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

void kernel_create (char* nombre_tabla, char* tipo_consistencia, int numero_particiones, int compaction_time){

	size_t tamanio_buffer = sizeof(int)*4 + strlen(nombre_tabla) + strlen(tipo_consistencia);
	void* buffer = serializar_request_create(nombre_tabla, tipo_consistencia, numero_particiones, compaction_time);
	prot_enviar_mensaje(socket_memoria, FUNCION_CREATE, tamanio_buffer, buffer);
	log_info(logger, "===========ENVIADO KERNEL/=======CREAATEEEE==============================");

	//printf("Holis :) hiciste un create desde kernel");
}


void imprimir_metricas() {
	printf("Implementando métricas version: %d\n", 1);

}



void kernel_describe(){
	// me trae todas la metadata de las tablas del FS
	// diferenciar si llega null o no en el argumento, C no permite argumentos opcionales
	printf("DESCRIBE DE TODAS LAS TABLAS DE FS::::::::::::::::::::");
}


void kernel_drop(char* nombre_tabla){
	// drop a la tabla de la tabla
	printf("DROP A LA TABLA:::::::::::::::::::::::::");
}

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
	size_t tamanio_del_buffer = strlen(nombre_tabla) + strlen(key) + strlen(value) + sizeof(int)*4 ;
		void *buffer = serializar_request_insert(nombre_tabla, key, value, epoch);
		prot_enviar_mensaje(socket_memoria, FUNCION_INSERT, tamanio_del_buffer, buffer);
		log_info(logger, "Insert enviado a memoria");

}

void kernel_create (char* nombre_tabla, char* tipo_consistencia, int numero_particiones, int compactation_time){
	printf("Holis :) hiciste un create desde kernel");
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

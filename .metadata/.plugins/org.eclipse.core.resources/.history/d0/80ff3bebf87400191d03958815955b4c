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

void kernel_create (char* nombre_tabla, char* tipo_consistencia, int numero_particiones, int compaction_time){

	size_t tamanio_buffer = sizeof(int)*4 + strlen(nombre_tabla) + strlen(tipo_consistencia);
	void* buffer = serializar_request_create(nombre_tabla, tipo_consistencia, numero_particiones, compaction_time);
	prot_enviar_mensaje(socket_memoria, FUNCION_CREATE, tamanio_buffer, buffer);
	log_info(logger, "===========ENVIADO KERNEL/=======CREAATEEEE==============================");

	//printf("Holis :) hiciste un create desde kernel");
}

void kernel_describe(char* nombre_tabla){
	printf("Hola mundo :) hiciste un describe a kernel (?\n");
	if(nombre_tabla == NULL){
		prot_enviar_mensaje(socket_memoria, FUNCION_DESCRIBE, 0, NULL);
	}
	else{
		prot_enviar_mensaje(socket_memoria, FUNCION_DESCRIBE, strlen(nombre_tabla), nombre_tabla);
	}
	log_info(logger, "Describe enviado a memoria");
	// me trae todas la metadata de las tablas del FS
	// diferenciar si llega null o no en el argumento, C no permite argumentos opcionales
}


void kernel_drop(char* nombre_tabla){
	printf("Hola mundo :) Dropeaste una tabla (?");
	prot_enviar_mensaje(socket_memoria, FUNCION_DROP,strlen(nombre_tabla), nombre_tabla);
	log_info(logger, "Drop enviado a memoria");
}

void kernel_journal(char* nombre_tabla){
	printf("Hola mundo :) Estás haciendo journaling (?");
}
void imprimir_metricas() {
	printf("Implementando métricas version: %d\n", 1);

}





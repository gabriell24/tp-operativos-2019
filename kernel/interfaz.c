#include "interfaz.h"

void kernel_select(int socket_memoria, char *tabla, uint16_t key) {
	size_t tamanio_del_buffer = sizeof(int) + strlen(tabla) + sizeof(uint16_t);
	void *buffer = serializar_request_select(tabla, key);
	prot_enviar_mensaje(socket_memoria, FUNCION_SELECT, tamanio_del_buffer, buffer);
	free(buffer);
	log_info(logger, "Select enviado a memoria");
}

void kernel_insert(int socket_memoria, char* nombre_tabla, uint16_t key, char* value, int epoch){
	size_t tamanio_del_buffer = strlen(nombre_tabla) + sizeof(uint16_t) + strlen(value) + sizeof(int)*3 ;
	void *buffer = serializar_request_insert(nombre_tabla, key, value, epoch);
	prot_enviar_mensaje(socket_memoria, FUNCION_INSERT, tamanio_del_buffer, buffer);
	free(buffer);
	log_info(logger, "Insert enviado a memoria");

}

void kernel_create(int socket_memoria, char* nombre_tabla, char* tipo_consistencia, int numero_particiones, int compaction_time){

	size_t tamanio_buffer = sizeof(int)*4 + strlen(nombre_tabla) + strlen(tipo_consistencia);
	void* buffer = serializar_request_create(nombre_tabla, tipo_consistencia, numero_particiones, compaction_time);
	prot_enviar_mensaje(socket_memoria, FUNCION_CREATE, tamanio_buffer, buffer);
	free(buffer);
	log_info(logger, "===========ENVIADO KERNEL/=======CREAATEEEE==============================");

	//printf("Holis :) hiciste un create desde kernel");
}

void kernel_describe(int socket_memoria, char* nombre_tabla){
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


void kernel_drop(int socket_memoria, char* nombre_tabla){
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

/*
 * Solo crear el proceso y moverlo
 */
void kernel_run(char *archivo) {
	//t_pcb *proceso = malloc(sizeof(t_pcb));
	FILE *file;
	char *ruta = string_new();
	//string_append_with_format(&ruta, "../otros/lql/%s", archivo);
	string_append(&ruta, archivo);
	file = fopen(ruta, "r");
	int total_de_lineas = 0;

	if(file != NULL) {
		log_debug(logger, "archivo abierto");
		/*char *linea = malloc(100*sizeof(char));
		memset(linea, 0, 100*sizeof(char));
		while(fgets(linea, 100, file)!=NULL) {

		}
		free(linea);*/
		crear_un_lql(false, ruta);
		fclose(file);
	}
	else {
		log_error(logger, "[RUN] Error: archivo no encontrado. Ruta: %s", ruta);
	}
	free(ruta);
}

void kernel_add(int numero_memoria, char *criterio) {
	agregar_memoria_a_criterio(criterio_from_string(criterio), numero_memoria);
}





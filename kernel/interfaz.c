#include "interfaz.h"

void kernel_select(char *tabla, char *clave) {
	printf("Holis soy Rodrigos");
	//Dummy
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



void kernel_describe(){
	// me trae todas la metadata de las tablas del FS
	// diferenciar si llega null o no en el argumento, C no permite argumentos opcionales
	printf("DESCRIBE DE TODAS LAS TABLAS DE FS::::::::::::::::::::");
}


void kernel_drop(char* nombre_tabla){
	// drop a la tabla de la tabla
	printf("DROP A LA TABLA:::::::::::::::::::::::::");
}

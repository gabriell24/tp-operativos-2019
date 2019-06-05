#include "planificacion.h"

void iniciar_listas_planificacion() {
	lista_nuevos = list_create();
	lista_ready = list_create();
	lista_exec = list_create();
}

void crear_execs() {
	for(int i = 0; i < kernel_config.multiprocesamiento; i++) {
		t_list *auxiliar = list_create();
		list_add(lista_exec, auxiliar);
		/*
		 * podria tener dentro de la lista, una esctructura, que marue ocupado
		 * o podria manejarme con el get y el remove
		 */
	}
}

void cargar_archivo_al_proceso() {

	FILE* file;
	char *path = string_new();
	//file = fopen(, "r");
}

void algoritmo() {
	int ut_consumidas = 0;
	while(ut_consumidas < kernel_config.multiprocesamiento) {
		/*
		 * tengo que tener una pcb donde guardo la prox linea a ejecutar
		 * debería tener el archivo cargado en memoria
		 * ejecuto por linea
		 */

	}
}

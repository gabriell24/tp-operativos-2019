#include "planificacion.h"

void iniciar_listas_planificacion() {
	lista_nuevos = list_create();
	lista_ready = list_create();
	lista_exec = list_create();
}

#include "criterios.h"

void iniciar_listas_de_criterios() {
	lista_sc = list_create();
	lista_shc = list_create();
	lista_ec = list_create();
}

void agregar_memoria_a_criterio(criterio criterio, int socket) {
	log_debug(logger, "Criterio recibido %s - Numero recibido: %d", criterio_to_string(criterio), socket);
	int *aux = malloc(sizeof(int));
	*aux = socket;
	switch(criterio) {
		case SC: {
			list_add(lista_sc, aux);
		} break;
		case SHC: {
			list_add(lista_shc, aux);
		} break;
		case EC: {
			list_add(lista_ec, aux);
		} break;
		default: log_error(logger, "Criterio no reconocido");
	}
	//free(aux);
}

void quitar_memoria_de_criterio(int socket) {
	quitar_de_lista(lista_sc, &socket);
	quitar_de_lista(lista_shc, &socket);
	quitar_de_lista(lista_ec, &socket);
}

void quitar_de_lista(t_list *lista, void *elemento) {
	bool _eliminar(void *elemento_lista) {
		if(*((int*)elemento_lista) == *((int*)elemento)) {
			free(elemento_lista);
			return true;
		}
		return false;
	}
	list_remove_by_condition(lista, _eliminar);
}

void imprimir_elementos(t_list *list, char *lista) {
	log_debug(logger, "COMIENZO - LISTA %s", lista);
	void _print(void *elemento) {
		printf("Elemento: %d\n", *((int*)elemento));
	}
	list_iterate(list, _print);
	log_debug(logger, "FINAL    - LISTA %s", lista);
}

void limpiar_listas() {
	list_clean_and_destroy_elements(lista_sc, (void*)free);
	list_clean_and_destroy_elements(lista_shc, (void*)free);
	list_clean_and_destroy_elements(lista_ec, (void*)free);
}

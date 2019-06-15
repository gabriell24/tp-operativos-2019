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
			if(list_size(lista_sc)) {
				log_error(logger, "[Error] El criterio SC solo puede asociarse a UNA memoria");
				return;
			}
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
		printf("\tElemento: %d\n", *((int*)elemento));
	}
	list_iterate(list, _print);
	log_debug(logger, "---------------------------------------");
}

void limpiar_listas() {
	list_clean_and_destroy_elements(lista_sc, (void*)free);
	list_clean_and_destroy_elements(lista_shc, (void*)free);
	list_clean_and_destroy_elements(lista_ec, (void*)free);
}

/*Explicación:
 * Tomo el abecedario sin la ñ, con 26 letras de total,
 * Tomo la primer letra de la tabla, la convierto a minúscula, el ascii para minúsculas comienza en el 97
 * A ese número le resto 96, para tener la correspondencia con el orden de las letras del abecedario
 * Luego armo grupos de letras con la mayor cantidad y un posible excedente.
 * Utilizando la misma logica, calculo a que grupo corresponde una letra, y le resto uno para tener base 0, con el list_get
 * Ejemplo: strlen(abecedario) = 26, tengo 7 memorias conectadas.
 * Letras por grupo = 4. abcd efgh ijkl mnop qrst uvwx yz
 * Tabla a usar YRIGOYEN, paso a minúscula yrigoyen, tomo la y.
 * a la y, le corresponde el orden 25, la divido por 4 letras x grupo, redondeo arriba = 7, pero necesito base 0 asi que uno.
 */
int elegir_memoria_shc(char *tabla) {
	char primer_letra = pasar_char_minuscula(tabla[0]);
	int nro_de_letra = primer_letra - 96; // a = 1, z = 26;
	int cantidad_letras_abecedario = 26; //Sin la enie
	int letras_por_grupo = redondear_hacia_arriba(cantidad_letras_abecedario, list_size(lista_shc));
	int grupo_correspondiente_base_cero = redondear_hacia_arriba(nro_de_letra, letras_por_grupo) - 1;
	log_debug(logger, "[Elegir SHC] Se calculó la memoria en la posición: %d", grupo_correspondiente_base_cero);
	//TODO MUTEX AT THIS POINT
	int *socket = list_get(lista_shc, grupo_correspondiente_base_cero);
	if(!socket) {
		log_error(logger, "No se pudo obtener una memoria criterio EC");
		return -1;
	}
	return *socket;
}

int pasar_char_minuscula(char letra) {
	if(letra < 96) letra += 32;
	return letra;
}

int elegir_memoria_ec() {
	int *socket = list_get(lista_ec, rand() % list_size(lista_ec));
	if(!socket) {
		log_error(logger, "No se pudo obtener una memoria criterio EC");
		return -1;
	}
	return *socket;
}

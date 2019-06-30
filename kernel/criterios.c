#include "criterios.h"

void iniciar_listas_de_criterios() {
	lista_sc = list_create();
	lista_shc = list_create();
	lista_ec = list_create();
}

void agregar_memoria_a_criterio(criterio criterio, int numero_memoria) {
	loguear(debug, logger, "Criterio recibido %s - Numero recibido: %d", criterio_to_string(criterio), socket);
	bool _buscar_memoria(t_memoria_conectada *memoria_conectada) {
		return memoria_conectada->nombre == numero_memoria;
	}
	t_memoria_conectada *memoria_conectada = list_find(tabla_gossip, (void *)_buscar_memoria);
	if(!memoria_conectada) {
		loguear(error, logger, "No encontré esa memoria conectada, configuraste el memoria.config?");
		return;
	} else {
		if(memoria_conectada->socket < 1)
			loguear(error, logger, "La memoria que querés agregar, aún no se conectó, valor de socket: %d", memoria_conectada->socket);
	}
	int *aux = malloc(sizeof(int));
	*aux = memoria_conectada->socket;
	switch(criterio) {
		case SC: {
			if(list_size(lista_sc)) {
				loguear(error, logger, "[Error] El criterio SC solo puede asociarse a UNA memoria");
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
		default: loguear(error, logger, "Criterio no reconocido");
	}
	//free(aux);
}

void quitar_memoria_de_criterio(int socket) {
	quitar_de_lista(lista_sc, &socket);
	quitar_de_lista(lista_shc, &socket);
	quitar_de_lista(lista_ec, &socket);
}

void quitar_de_lista(t_list *lista, int *elemento) {
	bool _eliminar(int *elemento_lista) {
		if(*elemento_lista == *elemento) {
			free(elemento_lista);
			return true;
		}
		return false;
	}
	list_remove_by_condition(lista, (void *)_eliminar);
}

void imprimir_elementos(t_list *list, char *lista) {
	loguear(debug, logger, "COMIENZO - LISTA %s", lista);
	void _print(void *elemento) {
		printf("\tElemento: %d\n", *((int*)elemento));
	}
	list_iterate(list, _print);
	loguear(debug, logger, "---------------------------------------");
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
	if(!list_size(lista_shc)) {
		loguear(error, logger, "Error: no se asocio una memoria al criterio SHC");
		return -1;
	}
	char primer_letra = pasar_char_minuscula(tabla[0]);
	int nro_de_letra = primer_letra - 96; // a = 1, z = 26;
	int cantidad_letras_abecedario = 26; //Sin la enie
	int letras_por_grupo = redondear_hacia_arriba(cantidad_letras_abecedario, list_size(lista_shc));
	int grupo_correspondiente_base_cero = redondear_hacia_arriba(nro_de_letra, letras_por_grupo) - 1;
	loguear(debug, logger, "[Elegir SHC] Se calculó la memoria en la posición: %d", grupo_correspondiente_base_cero);
	//TODO MUTEX AT THIS POINT
	int *socket = list_get(lista_shc, grupo_correspondiente_base_cero);
	if(!socket) {
		loguear(error, logger, "No se pudo obtener una memoria criterio EC");
		return -1;
	}
	return *socket;
}

int pasar_char_minuscula(char letra) {
	if(letra < 96) letra += 32;
	return letra;
}

int elegir_memoria_ec() {
	if(!list_size(lista_ec)) {
		loguear(error, logger, "Error: no se asocio una memoria al criterio EC");
		return -1;
	}
	int *socket = list_get(lista_ec, rand() % list_size(lista_ec));
	if(!socket) {
		loguear(error, logger, "No se pudo obtener una memoria criterio EC");
		return -1;
	}
	return *socket;
}

t_list *memorias_conectadas() {
	bool _buscar_conectadas(t_memoria_conectada *memoria_conectada) {
		return memoria_conectada->socket > 0;
	}
	t_list *memorias_conectadas = list_filter(tabla_gossip, (void *)_buscar_conectadas);
	if(!list_size(memorias_conectadas)) {
		loguear(error, logger, "ERROR: get_memoria() no hay memoria conectada, esto no debería pasar.");
		return NULL;
	}
	return memorias_conectadas;
}

/* Devuelve -1 cuando una tabla no está en el describe.
 * si no se le pasa una tabla, asume cualquiera de las memorias conectadas.
 * Por éxito devuelve el socket a donde se debería enviar un request.
 */
int get_memoria(char *tabla) {
	int socket_retorno = -1;
	if(!tabla) {
		t_list *memorias = memorias_conectadas();
		if(!list_size(memorias)) {
			return -1;
		}
		t_memoria_conectada *memoria_conectada = list_get(memorias, rand() % list_size(memorias));
		list_destroy(memorias);
		socket_retorno = memoria_conectada->socket;
	} else {
		bool _buscar_metadata(t_response_describe *describe) {
			return string_equals_ignore_case(describe->tabla, tabla);
		}
		t_response_describe *metadata = list_find(describe_tablas, (void *)_buscar_metadata);
		if(!metadata) {
			loguear(error, logger, "No existe metadata para %s", tabla);
			return -1;
		}
		switch(metadata->consistencia) {
			case SC: {
				if(!list_size(lista_sc)) {
					loguear(error, logger, "Error: no se asocio una memoria al criterio SC");
					return -1;
				}
				socket_retorno = *(int *)list_get(lista_sc, 0);
			} break;
			case SHC: {
				socket_retorno = elegir_memoria_shc(tabla);
			} break;
			case EC: {
				socket_retorno = elegir_memoria_ec();
			} break;
		}
	}
	return socket_retorno;
}

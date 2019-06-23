
#ifndef CRITERIOS_H_
#define CRITERIOS_H_

#include "../shared_library/utiles.h"
#include "kernel.h"
#include <commons/log.h>
#include <commons/collections/list.h>

void iniciar_listas_de_criterios();
void agregar_memoria_a_criterio(criterio criterio, int socket);
void quitar_memoria_de_criterio(int socket);
void quitar_de_lista(t_list *lista, int *elemento);
void imprimir_elementos(t_list *list, char *lista);
void limpiar_listas();
int elegir_memoria_shc(char *tabla);
int elegir_memoria_ec();
int pasar_char_minuscula(char letra);
int get_memoria(char *tabla);
#endif /* CRITERIOS_H_ */

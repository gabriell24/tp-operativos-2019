
#ifndef UTILES_H_
#define UTILES_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>
#include <commons/string.h>
#include <commons/collections/list.h>

typedef enum { SC, SHC, EC, INVALIDO } criterio;

int get_timestamp();
int calcular_particion(int particion,uint16_t key);
criterio criterio_from_string(char *string_criterio);
char *criterio_to_string(criterio t_criterio);
uint16_t string_to_int16(char *string);
int redondear_hacia_arriba(int numerador, int denominador);
int number_string(char *texto);

#endif /* UTILES_H_ */

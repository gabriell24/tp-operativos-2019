
#ifndef UTILES_H_
#define UTILES_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
//#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <readline/readline.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/list.h>

typedef enum { SC, SHC, EC, INVALIDO } criterio;
typedef enum { debug, info, warning, error } tipo_log;

uint64_t get_timestamp();
int calcular_particion(int particion,uint16_t key);
criterio criterio_from_string(char *string_criterio);
char *criterio_to_string(criterio t_criterio);
uint16_t string_to_int16(char *string);
uint64_t string_to_timestamp(char *string);
int redondear_hacia_arriba(int numerador, int denominador);
int number_string(char *texto);
int contar_items(char **lista);
void loguear(tipo_log tipo, t_log *logger, char* message, ...);

#endif /* UTILES_H_ */

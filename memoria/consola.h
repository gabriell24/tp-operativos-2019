#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#include "../shared_library/colores.h"
#include <commons/string.h>
#include <commons/log.h>
#include "interfaz.h"

#define SELECT "select"
#define INSERT "insert"
#define CREATE "create"
#define DESCRIBE "describe"
#define DROP "drop"
#define JOURNAL "journal"

typedef struct{
	char* keyword;
	char* ruta;
	char** _aux;
} t_consola;
t_log *logger;
bool consola_ejecuto_exit;

t_consola parse(char* linea);
void destruir_operacion(t_consola);

void consola();

#endif


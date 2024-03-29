#ifndef CONSOLA_H_
#define CONSOLA_H_

//Includes
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <dirent.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include "interfaz.h"
#include "criterios.h"
#include "../shared_library/colores.h"
#include "../shared_library/utiles.h"

//Defines
#define SELECT "select"
#define INSERT "insert"
#define CREATE "create"
#define DESCRIBE "describe"
#define DROP "drop"
#define JOURNAL "journal"
#define ADD "add"
#define RUN "run"
#define METRICS "metrics"

//Variables
typedef struct{
	char* keyword;
	char* ruta;
	char** _aux;
} t_consola;
t_log *logger;
bool consola_ejecuto_exit;

//Prototipos
t_consola parse(char* linea);
void destruir_operacion(t_consola);

void consola();

#endif


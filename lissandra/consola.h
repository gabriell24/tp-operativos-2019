#ifndef CONSOLA_H_
#define CONSOLA_H_

//Includes
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <dirent.h>
#include "../shared_library/colores.h"
#include "../shared_library/utiles.h"
#include <commons/string.h>
#include <commons/log.h>
#include "interfaz.h"
#include "filesystem.h"

//Defines
#define SELECT "select"
#define INSERT "insert"
#define CREATE "create"
#define DESCRIBE "describe"
#define DROP "drop"

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
void operaciones_disponibles();


#endif


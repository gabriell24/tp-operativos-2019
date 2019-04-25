#ifndef ESTRUCTURAS_COMPARTIDAS_H_
#define ESTRUCTURAS_COMPARTIDAS_H_

//Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "protocolo.h"

//Defines
typedef struct {
	char* tabla;
	char* key;
} t_request_select;
//Variables

//Prototipos
void* serializar_request_select(char *key, char*value);
t_request_select *deserializar_request_select(t_prot_mensaje *msje);
#endif

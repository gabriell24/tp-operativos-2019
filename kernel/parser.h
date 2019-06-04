#ifndef PARSER_H_
#define PARSER_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <commons/string.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include "consola.h"

#define token_select "SELECT"
#define token_insert "INSERT"
#define token_create "CREATE"
#define token_describe "DESCRIBE"
#define token_drop "DROP"
#define token_journal "JOURNAL"
#define token_add "ADD"
#define token_metrics "METRICS"

#define RETURN_ERROR t_parser ERROR={ .valido = false }; return ERROR

typedef enum { t_select, insert, create, describe, drop, journal, add, metrics } tokens;

typedef struct {
	bool valido;
	tokens token;
	union {
		struct {
			char *tabla;
			uint16_t key;
		} select;
		struct {
			char *tabla;
			uint16_t key;
			char *value;
			union {
				int timestamp;
			};
		} insert;
		struct {
			char *tabla;
			char *tipo_consistencia;
			int particiones;
			int compaction_time;
		} create;
		struct {
			union {
				char *tabla;
			};
		} describe;
		struct {
			char *tabla;
		} drop;
		struct {
		} journal;
		struct {
			int memoria;
			char *tipo_consistencia;
		} add;
		struct{
		}metrics;
	} parametros;
	char** _limp; //Para liberar cuando se usa el split
} t_parser;

char *comando_leido(tokens token);
t_parser leer(char*);
void destruir_parseo(t_parser operacion);
bool string_igual_case_sensitive(char*,char*);



#endif

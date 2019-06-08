#include "parser.h"

bool string_igual_case_sensitive(char* string, char* otro_string) {
	return strcmp(string, otro_string) == 0;
}

void destruir_parseo(t_parser operacion) {
	if (operacion._limp) {
		string_iterate_lines(operacion._limp, (void*) free);
		free(operacion._limp);
	}
}

t_parser leer(char* linea) {
	if(linea == NULL){
		fprintf(stderr, "No pude interpretar una linea nula\n");
		RETURN_ERROR;
	}

	t_parser retorno = {
		.valido = true
	};

	char* aux_linea = string_duplicate(linea);
	string_trim(&aux_linea);
	char** separador = string_n_split(aux_linea, 2, " ");

	char* token_leido = separador[0];
	char *argumentos = separador[1];

	retorno._limp = separador;

	if(string_igual_case_sensitive(token_leido, token_select)){
		char **sub_separador = string_n_split(argumentos, 2, " ");
		retorno.token = t_select;
		retorno.parametros.select.tabla = string_duplicate(sub_separador[0]);
		retorno.parametros.select.key = string_to_int16(sub_separador[1]);
		string_iterate_lines(sub_separador, (void*)free);
		free(sub_separador);
	}
	else if(string_igual_case_sensitive(token_leido, token_insert)){
		char** comillas = string_n_split(argumentos, 3, "\"");
		char** separador = string_n_split(comillas[0], 2, " ");

		char *tabla = separador[0];
		char *key = separador[1];
		char *value = comillas[1];
		char *timestamp = comillas[2];
		if (tabla == NULL || key == NULL || value == NULL) {
			log_error(logger, "Error: ejemplo de uso \'INSERT TABLA1 3 \"Mi nombre es kernel\" 1548421507\'");
		} else {
			retorno.token = insert;
			retorno.parametros.insert.tabla = string_duplicate(tabla);
			retorno.parametros.insert.key = string_to_int16(key);
			retorno.parametros.insert.value = string_duplicate(value);
			if(timestamp)
			retorno.parametros.insert.timestamp = atoi(timestamp);
		}
		string_iterate_lines(separador, (void*)free);
		free(separador);
		string_iterate_lines(comillas, (void*)free);
		free(comillas);
	}
	else if(string_igual_case_sensitive(token_leido, token_create)){
		char** separador = string_n_split(argumentos, 4, " ");
		char *tabla = (separador[0]);
		char *consistencia = (separador[1]);
		char *particiones = (separador[2]);
		char *compactacion = (separador[3]);
		if (tabla == NULL || consistencia == NULL || particiones == NULL || compactacion == NULL) {
			log_error(logger, "Error: ejemplo de uso \"CREATE TABLA1 SC 4 60000\"");
		} else {
			retorno.token = create;
			retorno.parametros.create.tabla = string_duplicate(tabla);
			retorno.parametros.create.tipo_consistencia = string_duplicate(consistencia);
			//TODO validar la consistencia leida: SC, SHC, EC
			retorno.parametros.create.particiones = atoi(particiones);
			retorno.parametros.create.compaction_time = atoi(compactacion);
		}
		string_iterate_lines(separador, (void*)free);
		free(separador);
	}
	else if(string_igual_case_sensitive(token_leido, token_describe)){
		retorno.token = describe;
		if(argumentos != NULL) retorno.parametros.describe.tabla = argumentos;
	}
	else if(string_igual_case_sensitive(token_leido, token_drop)){
		if (argumentos == NULL) {
			log_error(logger, "Error: ejemplo de uso \"DROP TABLA1\n");
		} else {
			retorno.token = drop;
			retorno.parametros.drop.tabla = argumentos;
		}

	}
	else if(string_igual_case_sensitive(token_leido, token_journal)){
		retorno.token = journal;
	}
	else if(string_igual_case_sensitive(token_leido, token_add)){
		char** separador = string_n_split(argumentos, 3, " ");
		char *numero = separador[0];
		char *criterio = separador[2];
		if(numero == NULL || criterio == NULL) {
			log_error(logger, "Error: ejemplo de uso \"ADD MEMORY [NÚMERO] TO [CRITERIO]\"");
		} else {
			retorno.token = add;
			retorno.parametros.add.memoria = atoi(numero);
			retorno.parametros.add.tipo_consistencia = string_duplicate(criterio);
		}
		string_iterate_lines(separador, (void*)free);
		free(separador);
	}
	else if(string_igual_case_sensitive(token_leido, token_metrics)){
			retorno.token = metrics;
	}
	else {
		fprintf(stderr, "No se encontro la palabra reservada <%s>\n", token_leido);
		RETURN_ERROR;
	}

	//string_iterate_lines(separador, (void*)free);
	//free(separador);
	free(aux_linea);
	return retorno;
}


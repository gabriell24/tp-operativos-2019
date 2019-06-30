#include "parser.h"

bool string_igual_case_sensitive(char* string, char* otro_string) {
	return strcmp(string, otro_string) == 0;
}

void destruir_parseo(t_parser operacion) {
	if(operacion.argumentos) {
		string_iterate_lines(operacion.argumentos, (void*) free);
		free(operacion.argumentos);
	}
	if(operacion.separador_espacios) {
		string_iterate_lines(operacion.separador_espacios, (void*) free);
		free(operacion.separador_espacios);
	}
	if(operacion.separador_comillas) {
		string_iterate_lines(operacion.separador_comillas, (void*) free);
		free(operacion.separador_comillas);
	}
}

t_parser leer(char* linea) {
	t_parser retorno = {
		.valido = false
	};

	if(linea == NULL){
		fprintf(stderr, "No pude interpretar una linea nula\n");
		return retorno;
	}

	char* aux_linea = string_duplicate(linea);
	string_trim(&aux_linea);
	char** separador = string_n_split(aux_linea, 2, " ");

	char* token_leido = separador[0];
	char *argumentos = separador[1];

	retorno.argumentos = separador;

	if(string_igual_case_sensitive(token_leido, token_select)){
		char **sub_separador = string_n_split(argumentos, 2, " ");
		char *fin_strtol;
		uint16_t key = strtol(sub_separador[1], &fin_strtol, 10);
		if(sub_separador[0] == NULL || sub_separador[1] == NULL) {
			loguear(error, logger, "Error: ejemplo de uso \'SELECT TABLA1 3\'");
		} else if(sub_separador[1] == fin_strtol || key < 0) {
			loguear(error, logger, "Error: key debe ser numérica y mayor igual a 0");
		} else {
			retorno.valido = true;
			retorno.token = t_select;
			retorno.parametros.select.tabla = sub_separador[0];
			retorno.parametros.select.key = string_to_int16(sub_separador[1]);
			//string_iterate_lines(sub_separador, (void*)free);
			//free(sub_separador);
			retorno.separador_espacios = sub_separador;
		}
	}
	else if(string_igual_case_sensitive(token_leido, token_insert)){
		char** comillas = string_n_split(argumentos, 3, "\"");
		char** separador = string_n_split(comillas[0], 2, " ");

		char *tabla = separador[0];
		char *fin_strtol;
		uint16_t key = strtol(separador[1], &fin_strtol, 10);
		char *value = comillas[1];
		char *string_timestamp = comillas[2];
		if (tabla == NULL || value == NULL) {
			//loguear(error, logger, "Error: ejemplo de uso \'INSERT TABLA1 3 \"Mi nombre es kernel\" 1548421507\'");
			loguear(error, logger, "Error: ejemplo de uso \'INSERT TABLA1 3 \"Mi nombre es kernel\" 1548421507\'");
		}
		if (separador[1] == fin_strtol || key < 0) {
			//loguear(error, logger, "Error: la key debe ser numérica y mayor igual a 0.");
			loguear(error, logger, "Error: ejemplo de uso \'INSERT TABLA1 3 \"Mi nombre es kernel\" 1548421507\'");
		} else {
			retorno.valido = true;
			retorno.token = insert;
			retorno.parametros.insert.tabla = tabla;
			retorno.parametros.insert.key = key;
			retorno.parametros.insert.value = value;
			int timestamp = !string_timestamp ? get_timestamp() : atoi(string_timestamp);
			retorno.parametros.insert.timestamp = timestamp;
		}
		/*string_iterate_lines(separador, (void*)free);
		free(separador);
		string_iterate_lines(comillas, (void*)free);
		free(comillas);*/
		retorno.separador_espacios = separador;
		retorno.separador_comillas = comillas;
	}
	else if(string_igual_case_sensitive(token_leido, token_create)){
		char** separador = string_n_split(argumentos, 4, " ");
		char *tabla = separador[0];
		char *consistencia = separador[1];
		char *particiones = separador[2];
		char *compactacion = separador[3];
		if (tabla == NULL || consistencia == NULL || particiones == NULL || compactacion == NULL) {
			loguear(error, logger, "Error: ejemplo de uso \"CREATE TABLA1 SC 4 60000\"");
			retorno.valido = false;
		} else {
			retorno.valido = true;
			retorno.token = create;
			retorno.parametros.create.tabla = tabla;
			retorno.parametros.create.tipo_consistencia = consistencia;
			//TODO validar la consistencia leida: SC, SHC, EC
			retorno.parametros.create.particiones = atoi(particiones);
			retorno.parametros.create.compaction_time = atoi(compactacion);
		}
		/*string_iterate_lines(separador, (void*)free);
		free(separador);*/
		retorno.separador_espacios = separador;
	}
	else if(string_igual_case_sensitive(token_leido, token_describe)){
		retorno.token = describe;
		retorno.valido = true;
		if(argumentos != NULL) {
			retorno.parametros.describe.tabla = argumentos;
		}
	}
	else if(string_igual_case_sensitive(token_leido, token_drop)){
		if (argumentos == NULL) {
			loguear(error, logger, "Error: ejemplo de uso \"DROP TABLA1\n");
		} else {
			retorno.valido = true;
			retorno.token = drop;
			retorno.parametros.drop.tabla = argumentos;
		}

	}
	else if(string_igual_case_sensitive(token_leido, token_journal)){
		retorno.valido = true;
		retorno.token = journal;
	}
	else if(string_igual_case_sensitive(token_leido, token_add)){
		char** separador = string_n_split(argumentos, 3, " ");
		char *numero = separador[0];
		char *criterio = separador[2];
		if(numero == NULL || criterio == NULL) {
			loguear(error, logger, "Error: ejemplo de uso \"ADD MEMORY [NÚMERO] TO [CRITERIO]\"");
			retorno.valido = false;
		} else {
			retorno.valido = true;
			retorno.token = add;
			retorno.parametros.add.memoria = atoi(numero);
			retorno.parametros.add.tipo_consistencia = criterio;
		}
		/*string_iterate_lines(separador, (void*)free);
		free(separador);*/
		retorno.separador_espacios = separador;
	}
	else if(string_igual_case_sensitive(token_leido, token_metrics)){
		retorno.valido = true;
		retorno.token = metrics;
	}
	else {
		fprintf(stderr, "No se encontro la palabra reservada <%s>\n", token_leido);
		retorno.valido = false;
	}

	//string_iterate_lines(separador, (void*)free);
	//free(separador);
	free(aux_linea);
	return retorno;
}


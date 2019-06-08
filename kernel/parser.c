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
	char** separador = string_n_split(aux_linea, 5, " ");

	char* token_leido = separador[0];

	retorno._limp = separador;

	if(string_igual_case_sensitive(token_leido, token_select)){
		retorno.token = t_select;
		retorno.parametros.select.tabla = separador[1];
		retorno.parametros.select.key = string_to_int16(separador[2]); //strtoul
	}
	else if(string_igual_case_sensitive(token_leido, token_insert)){
		retorno.token = insert;
		retorno.parametros.insert.tabla = separador[1];
		retorno.parametros.insert.key = string_to_int16(separador[2]);
		retorno.parametros.insert.value = separador[3];
		if(separador[4] != NULL) retorno.parametros.insert.timestamp = atoi(separador[4]);
	}
	else if(string_igual_case_sensitive(token_leido, token_create)){
		retorno.token = create;
		retorno.parametros.create.tabla = separador[1];
		retorno.parametros.create.tipo_consistencia = separador[2];
		//TODO validar la consistencia leida: SC, SHC, EC
		retorno.parametros.create.particiones = atoi(separador[3]);
		retorno.parametros.create.compaction_time = atoi(separador[4]);
	}
	else if(string_igual_case_sensitive(token_leido, token_describe)){
		retorno.token = describe;
		if(separador[1] != NULL) retorno.parametros.describe.tabla = separador[1];
	}
	else if(string_igual_case_sensitive(token_leido, token_drop)){
		retorno.token = drop;
		retorno.parametros.drop.tabla = separador[1];
	}
	else if(string_igual_case_sensitive(token_leido, token_journal)){
		retorno.token = journal;
	}
	else if(string_igual_case_sensitive(token_leido, token_add)){
		retorno.token = add;
		retorno.parametros.add.memoria = atoi(separador[1]);
		retorno.parametros.add.tipo_consistencia = separador[2];
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


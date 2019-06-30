#include "consola.h"

void consola() {
	char * linea;
	while (!consola_ejecuto_exit) {
		linea = readline(ANSI_COLOR_BLUE"Kernel$ "ANSI_COLOR_RESET);
		if (linea)
			add_history(linea);
		if (!strncmp(linea, "exit", 4)) {
			free(linea);
			consola_ejecuto_exit = true;
			break;
		}
		t_consola comando = parse(linea);
		free(linea);
		destruir_operacion(comando);
	}
}

void destruir_operacion(t_consola linea) {
	if (linea._aux) {
		string_iterate_lines(linea._aux, (void*) free);
		free(linea._aux);
	}
}

t_consola parse(char* linea) {
	if (linea == NULL || string_equals_ignore_case(linea, "")) {
		printf("Error: Debe ingresar una operacion conocida\n");
		t_consola error = { ._aux = NULL };
		return error;
	}

	char* auxLine = string_duplicate(linea);
	string_trim(&auxLine);

	char** split = string_n_split(auxLine, 2, " ");

	char* comando = split[0];
	char* argumentos = split[1];

	t_consola retorno = { ._aux = split };

	if (string_equals_ignore_case(comando, SELECT)) {
		if(!argumentos) {
			//log_error(logger, "Error: ejemplo de uso \"SELECT TABLA1 3\"");
			loguear(error, logger, "Error: ejemplo de uso \"SELECT TABLA1 3\"");
		} else {
			char** separador = string_n_split(argumentos, 2, " ");
			char *tabla = separador[0];
			char *key = separador[1];
			if (tabla == NULL || key == NULL) {
				loguear(error, logger, "Error: ejemplo de uso \"SELECT TABLA1 3\"");
			} else {
				//kernel_select(tabla, (uint16_t)strtoul(key, NULL, 10));
				//char *select_string = string_from_format("SELECT %s %d", tabla, string_to_int16(key));
				crear_un_lql(true, linea);
				//free(select_string);
			}
			string_iterate_lines(separador, (void*)free);
			free(separador);
		}
	}
	else if (string_equals_ignore_case(comando, INSERT)) {
		if(!argumentos) {
			loguear(error, logger, "Error: ejemplo de uso \'INSERT TABLA1 3 \"Mi nombre es kernel\" 1548421507\'");
		} else {
			char** comillas = string_n_split(argumentos, 3, "\"");
			char** separador = string_n_split(comillas[0], 2, " ");

			char *tabla = separador[0];
			char *key = separador[1];
			char *value = comillas[1];
			char *timestamp = comillas[2];
			if (tabla == NULL || key == NULL || value == NULL) {
				loguear(error, logger, "Error: ejemplo de uso \'INSERT TABLA1 3 \"Mi nombre es kernel\" 1548421507\'");
			} else {
				int epoch = timestamp != NULL ? atoi(timestamp) : get_timestamp();
				//kernel_insert(tabla, (uint16_t)strtoul(key, NULL, 10), value, epoch);
				crear_un_lql(true, linea);
			}
			string_iterate_lines(separador, (void*)free);
			free(separador);
			string_iterate_lines(comillas, (void*)free);
			free(comillas);
		}
	}
	else if (string_equals_ignore_case(comando, CREATE)) {
		if(!argumentos) {
			loguear(error, logger, "Error: ejemplo de uso \"CREATE TABLA1 SC 4 60000\"");
		} else {
			char** separador = string_n_split(argumentos, 4, " ");
			char *tabla = separador[0];
			char *consistencia = separador[1];
			char *particiones = separador[2];
			char *compactacion = separador[3];
			if (tabla == NULL || consistencia == NULL || particiones == NULL || compactacion == NULL) {
				loguear(error, logger, "Error: ejemplo de uso \"CREATE TABLA1 SC 4 60000\"");
			} else {
				//kernel_create(tabla, consistencia, atoi(particiones), atoi(compactacion));
				crear_un_lql(true, linea);
			}
			string_iterate_lines(separador, (void*)free);
			free(separador);
		}
	}
	else if (string_equals_ignore_case(comando, DESCRIBE)) {
		//kernel_describe(argumentos);
		crear_un_lql(true, linea);
	}
	else if (string_equals_ignore_case(comando, DROP)) {
		if (argumentos == NULL) {
			loguear(error, logger, "Error: ejemplo de uso \"DROP TABLA1\n");
		} else {
			//kernel_drop(argumentos);
			crear_un_lql(true, linea);
		}
	}
	else if (string_equals_ignore_case(comando, JOURNAL)) {
		//kernel_journal(argumentos);
		crear_un_lql(true, linea);
	}
	else if (string_equals_ignore_case(comando, ADD)) {
		/*char** separador = string_n_split(argumentos, 3, " ");
		char *numero = separador[0];
		char *criterio = separador[2];
		if(numero == NULL || criterio == NULL) {
			log_error(logger, "Error: ejemplo de uso \"ADD MEMORY [NÚMERO] TO [CRITERIO]\"");
		} else {
			log_debug(logger, "Criterio: %s, Numero: %s", criterio, numero);*/
			crear_un_lql(true, linea);
		//}
	}
	else if (string_equals_ignore_case(comando, RUN)) {
		if (argumentos == NULL) {
			loguear(error, logger, "Error: ejemplo de uso \"RUN UNLQL.txt\"");
		} else {
			kernel_run(argumentos);
		}
	}
	else if (string_equals_ignore_case(comando, METRICS)) {
		//imprimir_metricas();
		crear_un_lql(true, linea);
	}
	else if(string_equals_ignore_case(comando, "listas")) {
		imprimir_elementos(lista_sc, "SC");
		imprimir_elementos(lista_shc, "SHC");
		imprimir_elementos(lista_ec, "EC");
	}
	else if(string_equals_ignore_case(comando, "quitar")) {
		quitar_memoria_de_criterio(atoi(argumentos));
	}
	else {
		loguear(error, logger, "Error: No se encontro operacion tipeada.");
	}

	free(auxLine);

	return retorno;
}


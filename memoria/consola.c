#include "consola.h"

void consola(){
	char * linea;
	while(1) {
		linea = readline(ANSI_COLOR_RED"Memoria$ "ANSI_COLOR_RESET);
	    if(linea)
	    	add_history(linea);
	    if(!strncmp(linea, "exit", 4)) {
	    	free(linea);
	    	consola_ejecuto_exit = true;
	    	break;
	    }
	    t_consola comando = parse(linea);
		free(linea);
	    destruir_operacion(comando);
	 }
}

void destruir_operacion(t_consola linea){
	if(linea._aux){
		string_iterate_lines(linea._aux, (void*) free);
		free(linea._aux);
	}
}

t_consola parse(char* linea){
	if(linea == NULL || string_equals_ignore_case(linea,"")){
		printf("Error: Debe ingresar una operacion conocida\n");
		t_consola error={._aux=NULL};
		return error;
	}

	char* auxLine = string_duplicate(linea);
	string_trim(&auxLine);
	char** split = string_n_split(auxLine, 2, " ");

	char* comando = split[0];
	char* argumentos = split[1];

	t_consola retorno = {._aux = split};

	if(string_equals_ignore_case(comando, SELECT)) {
		if(!argumentos) {
			loguear(error, logger, "Error: ejemplo de uso \"SELECT TABLA1 3\"");
		} else {
			char** separador = string_n_split(argumentos, 2, " ");
			char *tabla = separador[0];
			char *key = separador[1];
			if (tabla == NULL || key == NULL) {
				loguear(error, logger, "Error: ejemplo de uso \"SELECT TABLA1 3\"");
			} else {
				char *respuesta = memoria_select(tabla, (uint16_t)strtoul(key, NULL, 10));
				loguear(info, logger, "[Select] %s", respuesta);
				free(respuesta);
			}
			string_iterate_lines(separador, (void*)free);
			free(separador);
		}
	}
	else if(string_equals_ignore_case(comando, INSERT)) {
		if(!argumentos) {
			loguear(error, logger, "Error: ejemplo de uso \'INSERT TABLA1 3 \"Mi nombre es memoria\"\'");
		} else {
			char** comillas = string_n_split(argumentos, 3, "\"");
			char** separador = string_n_split(comillas[0], 2, " ");

			char *tabla = separador[0];
			char *key = separador[1];
			char *value = comillas[1];
			if (tabla == NULL || key == NULL || value == NULL) {
				loguear(error, logger, "Error: ejemplo de uso \'INSERT TABLA1 3 \"Mi nombre es kernel\"\'");
			} else {
				uint64_t epoch = get_timestamp(); //TODO le podrian pasar el timestamp??
				memoria_insert(tabla, (uint16_t)strtoul(key, NULL, 10), value, epoch);
			}
			string_iterate_lines(separador, (void*)free);
			free(separador);
			string_iterate_lines(comillas, (void*)free);
			free(comillas);
		}
	}
	else if(string_equals_ignore_case(comando, CREATE)) {
		if(!argumentos) {
			loguear(error, logger, "Error: ejemplo de uso \"CREATE TABLA1 SC 4 60000\"");
		} else {
			char** separador = string_n_split(argumentos, 3, " ");
			char *tabla = separador[0];
			char *consistencia = separador[1];
			char *particiones = separador[2];
			char *compactacion = separador[3];
			if (tabla == NULL || consistencia == NULL || particiones == NULL || compactacion == NULL) {
				loguear(error, logger, "Error: ejemplo de uso \"CREATE TABLA1 SC 4 60000\"");
			} else {
				memoria_create(tabla, consistencia, atoi(particiones), atoi(compactacion));
			}
			string_iterate_lines(separador, (void*)free);
			free(separador);
		}
	}
	else if(string_equals_ignore_case(comando, DESCRIBE)) {
		memoria_describe(argumentos);
	}
	else if(string_equals_ignore_case(comando, DROP)) {
		if (argumentos == NULL) {
			loguear(error, logger, "Error: ejemplo de uso \"DROP TABLA1\n");
		} else {
			memoria_drop(argumentos);
		}
	}
	else if(string_equals_ignore_case(comando, JOURNAL)) {
		journal();
	}
	else if(string_equals_ignore_case(comando, PRINT)) {
		printear_memoria();
	}
	else if(string_equals_ignore_case(comando, GOSSIP)) {
		mostrar_tabla_gossip(tabla_gossip, logger);
	}
	else {
		loguear(error, logger, "Error: No se encontro operacion tipeada.\n");
	}

	free(auxLine);

	return retorno;
}


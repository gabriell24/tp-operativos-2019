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
			log_error(logger, "Error: ejemplo de uso \"SELECT TABLA1 3\"");
		} else {
			char** separador = string_n_split(argumentos, 2, " ");
			char *tabla = separador[0];
			char *key = separador[1];
			if (tabla == NULL || key == NULL) {
				log_error(logger, "Error: ejemplo de uso \"SELECT TABLA1 3\"");
			} else {
				//Invocar a la interfaz
			}
			string_iterate_lines(separador, (void*)free);
			free(separador);
		}
	}
	else if(string_equals_ignore_case(comando, INSERT)) {
		if(!argumentos) {
			log_error(logger, "Error: ejemplo de uso \'INSERT TABLA1 3 \"Mi nombre es memoria\"\'");
		} else {
			char** comillas = string_n_split(argumentos, 2, "\"");
			char** separador = string_n_split(comillas[0], 2, " ");

			char *tabla = separador[0];
			char *key = separador[1];
			char *value = comillas[1];
			if (tabla == NULL || key == NULL || value == NULL) {
				log_error(logger, "Error: ejemplo de uso \'INSERT TABLA1 3 \"Mi nombre es kernel\"\'");
			} else {
				int epoch = get_timestamp();
				//Invocar a la interfaz
			}
			string_iterate_lines(separador, (void*)free);
			free(separador);
			string_iterate_lines(comillas, (void*)free);
			free(comillas);
		}
	}
	else if(string_equals_ignore_case(comando, CREATE)) {
		if(!argumentos) {
			log_error(logger, "Error: ejemplo de uso \"CREATE TABLA1 SC 4 60000\"");
		} else {
			char** separador = string_n_split(argumentos, 3, " ");
			char *tabla = separador[0];
			char *consistencia = separador[1];
			char *particiones = separador[2];
			char *compactacion = separador[3];
			if (tabla == NULL || consistencia == NULL || particiones == NULL || compactacion == NULL) {
				log_error(logger, "Error: ejemplo de uso \"CREATE TABLA1 SC 4 60000\"");
			} else {
				//Invocar a la interfaz
			}
			string_iterate_lines(separador, (void*)free);
			free(separador);
		}
	}
	else if(string_equals_ignore_case(comando, DESCRIBE)) {
		//Validar acá y llamar a método en interfaz
	}
	else if(string_equals_ignore_case(comando, DROP)) {
		if (argumentos == NULL) {
			log_error(logger, "Error: ejemplo de uso \"DROP TABLA1\n");
		} else {
			//Invocar a la interfaz
		}
	}
	else if(string_equals_ignore_case(comando, JOURNAL)) {
		//Validar acá y llamar a método en interfaz
	}
	else {
		log_error(logger, "Error: No se encontro operacion tipeada.\n");
	}

	free(auxLine);

	return retorno;
}


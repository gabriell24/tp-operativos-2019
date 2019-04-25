#include "consola.h"

void consola(){
	char * linea;
	while(1) {
		linea = readline(ANSI_COLOR_BLUE"Kernel$ "ANSI_COLOR_RESET);
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
	char** split = string_n_split(auxLine,3," ");

	char* comando = split[0];
	char* argumento1 = split[1];
	char* argumento2 = split[2];
	char* argumento3 = split[3];
	char* argumento4 = split[4];

	t_consola retorno = {._aux = split};

	if(string_equals_ignore_case(comando, SELECT)) {
		if(argumento1 == NULL || argumento2 == NULL) {
			log_error(logger, "Error: ejemplo de uso \"SELECT TABLA1 3\"\n");
		}
		else {
			kernel_select(argumento1, argumento2);
		}
	}

	else if(string_equals_ignore_case(comando, INSERT)) {
		if(argumento1 == NULL || argumento2 == NULL || argumento3 == NULL) {
			log_error(logger, "Error: ejemplo de uso \"INSERT TABLA1 3 \"Mi nombre es kernel\" 1548421507\"\n");
		}
		else {
			int epoch = argumento4 != NULL ? atoi(argumento4) : get_timestamp();
			kernel_insert(argumento1, argumento2, argumento3, atoi(argumento4));
		}

	}
	else if(string_equals_ignore_case(comando, CREATE)) {
		if(argumento1 == NULL || argumento2 == NULL || argumento3 == NULL || argumento4 == NULL) {
			log_error(logger, "Error: ejemplo de uso \"CREATE TABLA1 SC 4 60000\"\n");
		}
		else {
			kernel_create(argumento1, argumento2, atoi(argumento3), atoi(argumento4));
		}
	}
	else if(string_equals_ignore_case(comando, DESCRIBE)) {
			kernel_describe(argumento1);

	}

	else if(string_equals_ignore_case(comando, DROP)) {
		if( argumento1 == NULL ) {
			log_error(logger, "Error: El argumento de drop no puede ser null\"\n");
			}
		else {
			kernel_drop(argumento1);
		}

	}
	else if(string_equals_ignore_case(comando, JOURNAL)) {
	}
	else if(string_equals_ignore_case(comando, ADD)) {
	}
	else if(string_equals_ignore_case(comando, RUN)) {
	}
	else if(string_equals_ignore_case(comando, METRICS)) {
		imprimir_metricas();
	}
	else {
		log_error(logger, "Error: No se encontro operacion tipeada.\n");
	}

	free(auxLine);

	return retorno;
}

int get_timestamp() {
	time_t result = time(NULL);
	if(result == ((time_t) -1)) {
		perror("no se pudo obtener epoch");
	}
	return result;
}


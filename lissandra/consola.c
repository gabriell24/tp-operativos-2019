#include "consola.h"

void consola(){
	char * linea;
	while(1) {
		linea = readline(ANSI_COLOR_CYAN"LISSANDRA$ "ANSI_COLOR_RESET);
	    if(linea)
	    	add_history(linea);
	    if(!strncmp(linea, "exit", 4)) {
	    	free(linea);
	    	finalizar_proceso_normal = true;
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

	t_consola retorno = {._aux = split};

	if(string_equals_ignore_case(comando, SELECT)) {
	}
	else if(string_equals_ignore_case(comando, INSERT)) {
	}
	else if(string_equals_ignore_case(comando, CREATE)) {
	}
	else if(string_equals_ignore_case(comando, DESCRIBE)) {
	}
	else if(string_equals_ignore_case(comando, DROP)) {
	}
	else {
		log_error(logger, "Error: No se encontro operacion tipeada.\n");
	}

	free(auxLine);

	return retorno;
}


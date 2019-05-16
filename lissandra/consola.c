#include "consola.h"

void consola(){
	char * linea;
	while(1) {
		linea = readline(ANSI_COLOR_CYAN"LISSANDRA$ "ANSI_COLOR_RESET);
	    if(linea)
	    	add_history(linea);
	    if(!strncmp(linea, "exit", 4)) {
	    	free(linea);
	    	finalizar_estructuras_fs();
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
	char** split = string_n_split(auxLine,5," ");

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
			fs_select(argumento1, (uint16_t)strtoul(argumento2, NULL, 10));
		}
	}
	else if(string_equals_ignore_case(comando, INSERT)) {
		if(argumento1 == NULL || argumento2 == NULL || argumento3 == NULL) {
			log_error(logger, "Error: ejemplo de uso \"INSERT TABLA1 3 \"Mi nombre es Lissandra\" 1548421507\"\n");
		}
		else {
			int epoch = argumento4 != NULL ? atoi(argumento4) : get_timestamp();
			fs_insert(argumento1, (uint16_t)strtoul(argumento2, NULL, 10), argumento3, epoch);
		}
	}
	else if(string_equals_ignore_case(comando, CREATE)) {
		if(argumento1 == NULL || argumento2 == NULL || argumento3 == NULL || argumento4 == NULL) {
			log_error(logger, "Error: ejemplo de uso \"CREATE TABLA1 SC 4 60000\"\n");
		}
		else {
			fs_create(argumento1, argumento2, atoi(argumento3), atoi(argumento4));
		}
	}
	else if(string_equals_ignore_case(comando, DESCRIBE)) {
		fs_describe(argumento1);
	}
	else if(string_equals_ignore_case(comando, DROP)) {
		if(argumento1 == NULL) {
			log_error(logger, "Error: ejemplo de uso \"DROP [NOMBRE_TABLA]\"\n");
		}
		else {
			fs_drop(argumento1);
		}
	}
	else {
		log_error(logger, "Error: No se encontro operacion tipeada.\n");
		operaciones_disponibles();
	}

	free(auxLine);

	return retorno;
}

void operaciones_disponibles() {
	printf("Listado de operaciones disponibles:\n");
	printf("select(2)\n");
	printf("insert(3, 4to epoch opcional)\n");
	printf("create(4)\n");
	printf("describe(0,1 para tabla específica)\n");
	printf("drop(1)\n");
}

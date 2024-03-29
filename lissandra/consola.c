#include "consola.h"

void consola(){
	char * linea;

	while(1) {
		linea = readline(ANSI_COLOR_CYAN"LISSANDRA$ "ANSI_COLOR_RESET);
	    if(linea)
	    	add_history(linea);
	    if(!strncmp(linea, "exit", 4)) {
	    	free(linea);
	    	//finalizar_estructuras_fs();
	    	pthread_mutex_lock(&mutex_ejecuto_exit);
	    	consola_ejecuto_exit = true;
			pthread_mutex_unlock(&mutex_ejecuto_exit);
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
				char *resultado_string = fs_select(tabla, (uint16_t)strtoul(key, NULL, 10));
				/*char *resultado_string = malloc(strlen(resultado_stream)+1);
				memset(resultado_string, 0, strlen(resultado_stream)+1);
				memcpy(resultado_string, resultado_stream, strlen(resultado_stream));
				resultado_string[strlen(resultado_string)] = '\0';*/
				loguear(info, logger, "[Select] Resultado: %s", resultado_string);
				if(!string_equals_ignore_case(resultado_string, ERROR_KEY_NO_ENCONTRADA) &&
						!string_equals_ignore_case(resultado_string, ERROR_NO_EXISTE_TABLA)) {
					free(resultado_string);
				}
				//free(resultado_string);
			}
			string_iterate_lines(separador, (void*)free);
			free(separador);
		}
	}
	else if(string_equals_ignore_case(comando, INSERT)) {
		if(!argumentos) {
			loguear(error, logger, "Error: ejemplo de uso \'INSERT TABLA1 3 \"Mi nombre es Lissandra\" 1548421507\'");
		} else {
			char** comillas = string_n_split(argumentos, 3, "\"");
			char** separador = string_n_split(comillas[0], 2, " ");

			char *tabla = separador[0];
			char *key = separador[1];
			char *value = comillas[1];
			if (tabla == NULL || key == NULL || value == NULL) {
				loguear(error, logger, "Error: ejemplo de uso \'INSERT TABLA1 3 \"Mi nombre es Lissandra\" 1548421507\'");
			} else {
				uint64_t epoch = comillas[2] != NULL ? string_to_timestamp(comillas[2]) : get_timestamp();
				fs_insert(tabla, (uint16_t)strtoul(key, NULL, 10), value, epoch);
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
			char** separador = string_n_split(argumentos, 4, " ");
			char *tabla = separador[0];
			char *consistencia = separador[1];
			char *particiones = separador[2];
			char *compactacion = separador[3];
			if (tabla == NULL || consistencia == NULL || particiones == NULL || compactacion == NULL) {
				loguear(error, logger, "Error: ejemplo de uso \"CREATE TABLA1 SC 4 60000\"");
			} else {
				char*respuesta = fs_create(tabla, consistencia, atoi(particiones), atoi(compactacion));
				loguear(info, logger, "SOLO RESPUESTA: %s :::::::::::::::::::::", respuesta );
				free(respuesta);
			}
			string_iterate_lines(separador, (void*)free);
			free(separador);
		}
	}
	else if(string_equals_ignore_case(comando, DESCRIBE)) {
		t_list *tablas = fs_describe(argumentos);
		if(tablas)
		imprimir_datos_describe(tablas);
	}
	else if(string_equals_ignore_case(comando, DROP)) {
		if(!argumentos) {
			loguear(error, logger, "Error: ejemplo de uso \"DROP [NOMBRE_TABLA]\"\n");
		}
		else {
			fs_drop(argumentos);
		}
	}
	else if(string_equals_ignore_case(comando, MEMTABLE)) {
		printear_memtable();
	}
	else if(string_equals_ignore_case(comando, DUMP)) {
		dumpear();
	}
	else if(string_equals_ignore_case(comando, COMPACTAR)) {
		efectuar_compactacion(argumentos);
	}
	else if(string_equals_ignore_case(comando, EXPORTAR)) {
		exportar();
	}
	else {
		loguear(error, logger, "Error: No se encontro operacion tipeada.\n");
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

bool finalizo_proceso() {
	bool retorno = false;
	pthread_mutex_lock(&mutex_ejecuto_exit);
	retorno = consola_ejecuto_exit;
	pthread_mutex_unlock(&mutex_ejecuto_exit);
	return retorno;
}

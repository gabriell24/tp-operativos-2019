#include "utiles.h"

/* Usar esta
 * ulong get_timestamp() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
}*/

/* Esta no se usa más */
int get_timestamp() {
	time_t result = time(NULL);
	if(result == ((time_t) -1)) {
		perror("no se pudo obtener epoch");
	}
	return result;
}

int calcular_particion(int particion,uint16_t key) {
	return key%particion;
}

criterio criterio_from_string(char *string_criterio) {
	criterio retorno = INVALIDO;
	if(string_equals_ignore_case(string_criterio, "SC")) {
		retorno = SC;
	} else if(string_equals_ignore_case(string_criterio, "SHC")) {
		retorno = SHC;
	}
	else if(string_equals_ignore_case(string_criterio, "EC")) {
		retorno = EC;
	}
	else {
	printf("[Error] Criterio no reconocido\n");
	}
	return retorno;
}

char *criterio_to_string(criterio t_criterio) {
	switch(t_criterio) {
	case SC: return "SC";
	case SHC: return "SHC";
	case EC: return "EC";
	}
	return "[Error] Criterio no reconocido";
}

uint16_t string_to_int16(char *string) {
	return (uint16_t)strtoul(string, NULL, 10);
}

int redondear_hacia_arriba(int numerador, int denominador) {
	return (numerador/denominador) + ((numerador%denominador)!=0);
}

int number_string(char *texto){
	char *aux = string_new();
	int posiciones = 0;
	while(*(texto + posiciones) != '\0'){
		if(*(texto + posiciones) >= 48 && *(texto + posiciones) <= 57){
			string_append_with_format(&aux, "%d", *(texto + posiciones) - 48);
		}
		posiciones++;
	}
	int numero = atoi(aux);
	free(aux);
	return numero;
}

int contar_items(char **lista) {
	int total = 0;
	while(lista[total] != NULL) total++;
	return total;
}

void loguear(tipo_log tipo, t_log *logger, char* message, ...) {
	int redisplay = (rl_readline_state & RL_STATE_READCMD) > 0;
	int saved_point;
	char *saved_line;
	if(redisplay) {
		saved_point = rl_point;
		saved_line = rl_copy_text(0, rl_end);
		rl_replace_line("", 0);
		rl_redisplay();
	}

	va_list arguments;
	va_start(arguments, message);
	char *mensaje = string_from_vformat(message, arguments);

	switch(tipo) {
		case info: {
			log_info(logger, mensaje);
		} break;
		case debug: {
			log_debug(logger, mensaje);
		} break;
		case warning: {
			log_warning(logger, mensaje);
		} break;
		case error: {
			log_error(logger, mensaje);
		}
	}
	free(mensaje);
	va_end(arguments);

	if(redisplay) {
		rl_restore_prompt();
		//printf("need hack :%d\n", need_hack);
		if(redisplay) rl_replace_line(saved_line, 0);
		rl_point = saved_point;
		rl_redisplay();
		free(saved_line);
	}
}

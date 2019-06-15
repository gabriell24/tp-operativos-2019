#include "utiles.h"

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

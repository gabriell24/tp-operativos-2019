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
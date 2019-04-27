#include "estructuras_compartidas.h"

void* serializar_request_select(char *tabla, char*key) {
	int largo_tabla = strlen(tabla);
	int largo_key = strlen(key);
	size_t tamanio_del_paquete = sizeof(int)*2 + largo_tabla + largo_key;

	void *buffer = malloc(tamanio_del_paquete);
	int desplazamiento = 0;
	memset(buffer, 0, tamanio_del_paquete);

	//Copio el valor de largo de key
	memcpy(buffer+desplazamiento, &largo_tabla, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(buffer+desplazamiento, tabla, largo_tabla);
	desplazamiento += largo_tabla;

	//Copio el valor de largo de value
	memcpy(buffer+desplazamiento, &largo_key, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(buffer+desplazamiento, key, largo_key);
	desplazamiento += largo_key;

	return buffer;

}

t_request_select *deserializar_request_select(t_prot_mensaje *mensaje) {
	int largo_tabla, largo_key, desplazamiento;
	char *tabla,*key;
	desplazamiento = 0;
	size_t tamanio_del_paquete = mensaje->tamanio_total - sizeof(t_header);
	t_request_select *retorno = malloc(sizeof(t_request_select));
	memcpy(&largo_tabla, mensaje->payload+desplazamiento, sizeof(int));
	tabla = malloc(largo_tabla+1);
	memset(tabla, 0, largo_tabla+1);
	desplazamiento += sizeof(int);
	memcpy(tabla, mensaje->payload+desplazamiento, largo_tabla);
	tabla[largo_tabla] = '\0';
	//El arreglo está en base 0, largo tabla comienza en uno, entonces
	// estoy pasado de la última letra
	retorno->tabla = tabla;


	desplazamiento += largo_tabla;
	memcpy(&largo_key, mensaje->payload+desplazamiento, sizeof(int));
	key = malloc(largo_key+1);
	memset(key, 0, largo_key+1);
	desplazamiento += sizeof(int);
	memcpy(key, mensaje->payload+desplazamiento, largo_key);
	key[largo_key] = '\0';
	retorno->key = key;

	desplazamiento += largo_key;
	printf("Sizeof buffer: %d, desplazamiento: %d", tamanio_del_paquete, desplazamiento);

	return retorno;

}



//Insert
void* serializar_request_insert(char* nombre_tabla, char* key, char* value, int epoch) {
	int largo_nombre_de_tabla = strlen(nombre_tabla);
	int largo_key = strlen(key);
	int largo_value = strlen(value);
	size_t tamanio_del_paquete = ((largo_nombre_de_tabla + largo_key + largo_value)*sizeof(char)) + (sizeof(int)*4);

	void *buffer = malloc(tamanio_del_paquete);
	int desplazamiento = 0;
	memset(buffer, 0, tamanio_del_paquete);

	//Copio el valor de largo de key
	memcpy(buffer+desplazamiento, &largo_nombre_de_tabla, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(buffer+desplazamiento, nombre_tabla, largo_nombre_de_tabla);
	desplazamiento += largo_nombre_de_tabla;


	//Copio el valor de largo de key
	memcpy(buffer+desplazamiento, &largo_key, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(buffer+desplazamiento, key, largo_key);
	desplazamiento += largo_key;



	//Copio el valor de largo de value
	memcpy(buffer+desplazamiento, &largo_value, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(buffer+desplazamiento, value, largo_value);
	desplazamiento += largo_value;


	//Copio el valor de largo de timestamp
	memcpy(buffer+desplazamiento, &epoch, sizeof(int));
	desplazamiento += sizeof(int);


	return buffer;

}

t_request_insert *deserializar_request_insert(t_prot_mensaje *mensaje) {
	int epoch, desplazamiento, largo_nombre_de_tabla, largo_key, largo_value;
	char *value, *key, *nombre_tabla;
	desplazamiento = 0;
	size_t tamanio_del_paquete = mensaje->tamanio_total - sizeof(t_header);
	t_request_insert *retorno = malloc(sizeof(t_request_insert));

	memset(retorno, 0, sizeof(t_request_insert));
	memcpy(&largo_nombre_de_tabla, mensaje->payload+desplazamiento, sizeof(int));
	nombre_tabla = malloc(largo_nombre_de_tabla+1);
	memset(nombre_tabla, 0, largo_nombre_de_tabla+1);
	desplazamiento += sizeof(int);
	memcpy(nombre_tabla, mensaje->payload+desplazamiento, largo_nombre_de_tabla);
	nombre_tabla[largo_nombre_de_tabla] = '\0';
	//El arreglo está en base 0, largo tabla comienza en uno, entonces
	// estoy pasado de la última letra
	retorno->nombre_tabla = nombre_tabla;
	desplazamiento += largo_nombre_de_tabla;


	memcpy(&largo_key, mensaje->payload+desplazamiento, sizeof(int));
	printf("Tamaño de key %d", largo_key);
	key = malloc(largo_key);
	memset(key, 0, largo_key);
	desplazamiento += sizeof(int);
	memcpy(key, mensaje->payload+desplazamiento, largo_key);
	printf("Variable: %s", key);
	key[largo_key] = '\0';
	//TODO: REVISAR PORQUE NO PUEDO ASIGNARLO CORRECTAMENTE (PARA ESO USAMOS STRDUP)
	retorno->key = strdup(key);
	printf("PrintF de retorno key %s", retorno->key);
	desplazamiento += largo_key;


	memcpy(&largo_value, mensaje->payload+desplazamiento, sizeof(int));
	value = malloc(largo_value+1);
	memset(key, 0, largo_value+1);
	desplazamiento += sizeof(int);
	memcpy(value, mensaje->payload+desplazamiento, largo_value);
	value[largo_value] = '\0';
	retorno->value = value;
	desplazamiento += largo_value;


	memcpy(&epoch, mensaje->payload+desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
	retorno->epoch = epoch;

	printf("Sizeof buffer: %d, desplazamiento: %d", tamanio_del_paquete, desplazamiento);

	return retorno;

}

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

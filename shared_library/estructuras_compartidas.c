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

void* serializar_request_create(char* nombre_tabla, char* tipo_consistencia, int numero_particiones, int compaction_time){
	int largo_nombre_tabla = strlen(nombre_tabla);
	int largo_tipo_consistencia = strlen(tipo_consistencia);

	size_t tamanio_paquete = sizeof(int)*4 + largo_nombre_tabla + largo_tipo_consistencia;

	void* buffer = malloc(tamanio_paquete);
	int desplazamiento = 0;
	memset(buffer, 0, tamanio_paquete);

	memcpy(buffer+desplazamiento, &largo_nombre_tabla, sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(buffer+desplazamiento, nombre_tabla, largo_nombre_tabla);
	desplazamiento+= largo_nombre_tabla;

	memcpy(buffer+desplazamiento, &largo_tipo_consistencia, sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(buffer+desplazamiento, tipo_consistencia, largo_tipo_consistencia);
	desplazamiento += largo_tipo_consistencia;

	memcpy(buffer+desplazamiento, &numero_particiones, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(buffer+desplazamiento, &compaction_time, sizeof(int));
	desplazamiento += sizeof(int);

	if (tamanio_paquete != desplazamiento)
	{
		printf("::::::::::::::::::::MAL SERIALIZADOOOOOOOOOOOOOOOOOOOO::::::::::::::::!!!!!!!!!1!!11!!111!!!!1111111!!!!!!!");
		exit(1);
	}

	return buffer;
}

t_request_create *deserializar_request_create(t_prot_mensaje *mensaje){
	int largo_nombre_tabla, largo_tipo_consistencia, desplazamiento, numero_particiones, compaction_time;
	char *nombre_tabla, *tipo_consistencia;
	desplazamiento = 0;
	size_t tamanio_paquete = mensaje->tamanio_total - sizeof(t_header);
	t_request_create *retorno = malloc(sizeof(t_request_create));

	memcpy(&largo_nombre_tabla, mensaje->payload+desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
	nombre_tabla = malloc(largo_nombre_tabla+1);
	memset(nombre_tabla, 0, largo_nombre_tabla+1);
	memcpy(nombre_tabla, mensaje->payload+desplazamiento, largo_nombre_tabla);
	desplazamiento += largo_nombre_tabla;
	nombre_tabla[largo_nombre_tabla] = '\0';
	retorno->nombre_tabla = nombre_tabla;

	memcpy(&largo_tipo_consistencia,mensaje->payload+desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
	tipo_consistencia = malloc(largo_tipo_consistencia +1);
	memset(tipo_consistencia,0, largo_tipo_consistencia+1);
	memcpy(tipo_consistencia, mensaje->payload+desplazamiento, largo_tipo_consistencia);
	desplazamiento += largo_tipo_consistencia;
	tipo_consistencia[largo_tipo_consistencia] = '\0';
	retorno->tipo_consistencia = tipo_consistencia;

	//deserializo int

	memcpy(&numero_particiones, mensaje->payload+desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
	retorno->numero_particiones = numero_particiones;

	memcpy(&compaction_time, mensaje->payload+desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
	retorno->compaction_time = compaction_time;


	return retorno;
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
	//char *value, *key, *nombre_tabla;
	desplazamiento = 0;
	size_t tamanio_del_paquete = mensaje->tamanio_total - sizeof(t_header);
	t_request_insert *retorno = malloc(sizeof(t_request_insert));

	//memset(retorno, 0, sizeof(t_request_insert));
	memcpy(&largo_nombre_de_tabla, mensaje->payload+desplazamiento, sizeof(int));
	retorno->nombre_tabla = malloc(largo_nombre_de_tabla+1);
	memset(retorno->nombre_tabla, 0, largo_nombre_de_tabla+1);
	desplazamiento += sizeof(int);
	memcpy(retorno->nombre_tabla, mensaje->payload+desplazamiento, largo_nombre_de_tabla);
	retorno->nombre_tabla[largo_nombre_de_tabla] = '\0';
	//El arreglo está en base 0, largo tabla comienza en uno, entonces
	// estoy pasado de la última letra
	//retorno->nombre_tabla = nombre_tabla;
	desplazamiento += largo_nombre_de_tabla;


	memcpy(&largo_key, mensaje->payload+desplazamiento, sizeof(int));
	retorno->key = malloc(largo_key+1);
	memset(retorno->key, 0, largo_key+1);
	desplazamiento += sizeof(int);
	memcpy(retorno->key, mensaje->payload+desplazamiento, largo_key);
	retorno->key[largo_key] = '\0';
	desplazamiento += largo_key;


	memcpy(&largo_value, mensaje->payload+desplazamiento, sizeof(int));
	retorno->value = malloc(largo_value+1);
	memset(retorno->value, 0, largo_value+1);
	desplazamiento += sizeof(int);
	memcpy(retorno->value, mensaje->payload+desplazamiento, largo_value);
	retorno->value[largo_value] = '\0';
	//retorno->value = value;
	desplazamiento += largo_value;


	memcpy(&epoch, mensaje->payload+desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
	retorno->epoch = epoch;

	printf("Sizeof buffer: %d, desplazamiento: %d", tamanio_del_paquete, desplazamiento);

	return retorno;

}

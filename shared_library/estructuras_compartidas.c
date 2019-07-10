#include "estructuras_compartidas.h"

void* serializar_request_select(char *tabla, uint16_t key) {
	int largo_tabla = strlen(tabla);
	size_t tamanio_del_paquete = sizeof(int) + largo_tabla + sizeof(uint16_t);

	void *buffer = malloc(tamanio_del_paquete);
	int desplazamiento = 0;
	memset(buffer, 0, tamanio_del_paquete);

	//Copio el valor de largo de key
	memcpy(buffer+desplazamiento, &largo_tabla, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(buffer+desplazamiento, tabla, largo_tabla);
	desplazamiento += largo_tabla;

	//Copio el valor de largo de value
	memcpy(buffer+desplazamiento, &key , sizeof(uint16_t));
	desplazamiento += sizeof(uint16_t);

	printf("SALIMOS DE SERIALIZAR_REQUEST_SELECT\n");

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

	if (tamanio_paquete != desplazamiento) {
		printf("::::::::::::::::::::MAL SERIALIZADOOOOOOOOOOOOOOOOOOOO::::::::::::::::!!!!!!!!!1!!11!!111!!!!1111111!!!!!!!\n");
		exit(1);
	}

	return buffer;
}

t_request_create *deserializar_request_create(t_prot_mensaje *mensaje){
	int largo_nombre_tabla, largo_tipo_consistencia, desplazamiento, numero_particiones, compaction_time;

	desplazamiento = 0;
	size_t tamanio_paquete = mensaje->tamanio_total - sizeof(t_header);
	t_request_create *retorno = malloc(sizeof(t_request_create));

	memcpy(&largo_nombre_tabla, mensaje->payload+desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
	retorno->nombre_tabla = malloc(largo_nombre_tabla+1);
	memset(retorno->nombre_tabla, 0, largo_nombre_tabla+1);
	memcpy(retorno->nombre_tabla, mensaje->payload+desplazamiento, largo_nombre_tabla);
	desplazamiento += largo_nombre_tabla;
	retorno->nombre_tabla[largo_nombre_tabla] = '\0';


	memcpy(&largo_tipo_consistencia,mensaje->payload+desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
	retorno->tipo_consistencia = malloc(largo_tipo_consistencia +1);
	memset(retorno->tipo_consistencia,0, largo_tipo_consistencia+1);
	memcpy(retorno->tipo_consistencia, mensaje->payload+desplazamiento, largo_tipo_consistencia);
	desplazamiento += largo_tipo_consistencia;
	retorno->tipo_consistencia[largo_tipo_consistencia] = '\0';

	//deserializo int

	memcpy(&numero_particiones, mensaje->payload+desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
	retorno->numero_particiones = numero_particiones;

	memcpy(&compaction_time, mensaje->payload+desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
	retorno->compaction_time = compaction_time;

	if (tamanio_paquete != desplazamiento) {
		printf("::::::::::::::::::::MAL SERIALIZADOOOOOOOOOOOOOOOOOOOO::::::::::::::::!!!!!!!!!1!!11!!111!!!!1111111!!!!!!!\n");
		exit(1);
	}

	return retorno;
}

t_request_select *deserializar_request_select(t_prot_mensaje *mensaje) {
	int largo_tabla, desplazamiento;

	desplazamiento = 0;
	size_t tamanio_del_paquete = mensaje->tamanio_total - sizeof(t_header);
	t_request_select *retorno = malloc(sizeof(t_request_select));

	memcpy(&largo_tabla, mensaje->payload+desplazamiento, sizeof(int));
	retorno->tabla = malloc(largo_tabla+1);
	memset(retorno->tabla, 0, largo_tabla+1);
	desplazamiento += sizeof(int);

	memcpy(retorno->tabla, mensaje->payload+desplazamiento, largo_tabla);
	retorno->tabla[largo_tabla] = '\0';
	//El arreglo está en base 0, largo tabla comienza en uno, entonces
	// estoy pasado de la última letra

	desplazamiento += largo_tabla;
	memcpy(&retorno->key, mensaje->payload+desplazamiento, sizeof(uint16_t));
	desplazamiento += sizeof(uint16_t);

	if (tamanio_del_paquete != desplazamiento) {
		printf("::::::::::::::::::::MAL SERIALIZADOOOOOOOOOOOOOOOOOOOO::::::::::::::::!!!!!!!!!1!!11!!111!!!!1111111!!!!!!!\n");
		exit(1);
	}

	return retorno;

}



//Insert
void* serializar_request_insert(char* nombre_tabla, uint16_t key, char* value, uint64_t epoch) {
	int largo_nombre_de_tabla = strlen(nombre_tabla);
	int largo_value = strlen(value);
	size_t tamanio_del_paquete = ((largo_nombre_de_tabla + largo_value)*sizeof(char)) + (sizeof(int)*2 + sizeof(uint64_t) + sizeof(uint16_t));

	void *buffer = malloc(tamanio_del_paquete);
	int desplazamiento = 0;
	memset(buffer, 0, tamanio_del_paquete);

	//Copio el valor de largo de key
	memcpy(buffer+desplazamiento, &largo_nombre_de_tabla, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(buffer+desplazamiento, nombre_tabla, largo_nombre_de_tabla);
	desplazamiento += largo_nombre_de_tabla;


	//Copio el valor de largo de key
	memcpy(buffer+desplazamiento, &key, sizeof(uint16_t));
	desplazamiento += sizeof(uint16_t);



	//Copio el valor de largo de value
	memcpy(buffer+desplazamiento, &largo_value, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(buffer+desplazamiento, value, largo_value);
	desplazamiento += largo_value;


	//Copio el valor de largo de timestamp
	memcpy(buffer+desplazamiento, &epoch, sizeof(uint64_t));
	desplazamiento += sizeof(uint64_t);

	if (tamanio_del_paquete != desplazamiento) {
		printf("::::::::::::::::::::MAL SERIALIZADOOOOOOOOOOOOOOOOOOOO::::::::::::::::!!!!!!!!!1!!11!!111!!!!1111111!!!!!!!\n");
		exit(1);
	}


	return buffer;

}

t_request_insert *deserializar_request_insert(t_prot_mensaje *mensaje) {
	int desplazamiento, largo_nombre_de_tabla, largo_value;
	uint64_t epoch;
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


	memcpy(&retorno->key, mensaje->payload+desplazamiento, sizeof(uint16_t));
	desplazamiento += sizeof(uint16_t);


	memcpy(&largo_value, mensaje->payload+desplazamiento, sizeof(int));
	retorno->value = malloc(largo_value+1);
	memset(retorno->value, 0, largo_value+1);
	desplazamiento += sizeof(int);
	memcpy(retorno->value, mensaje->payload+desplazamiento, largo_value);
	retorno->value[largo_value] = '\0';
	//retorno->value = value;
	desplazamiento += largo_value;


	memcpy(&epoch, mensaje->payload+desplazamiento, sizeof(uint64_t));
	desplazamiento += sizeof(uint64_t);
	retorno->epoch = epoch;


	if (tamanio_del_paquete != desplazamiento) {
		printf("::::::::::::::::::::MAL SERIALIZADOOOOOOOOOOOOOOOOOOOO::::::::::::::::!!!!!!!!!1!!11!!111!!!!1111111!!!!!!!\n");
		exit(1);
	}

	return retorno;

}

void *serializar_response_describe(size_t tamanio_del_buffer, t_list *tablas) {
	void *buffer = malloc(tamanio_del_buffer);
	memset(buffer, 0, tamanio_del_buffer);
	int desplazamiento = 0;
	void _cargar_en_buffer(t_response_describe *tabla) {
		int largo_nombre_tabla = strlen(tabla->tabla);
		memcpy(buffer+desplazamiento, &largo_nombre_tabla, sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(buffer+desplazamiento, tabla->tabla, largo_nombre_tabla);
		desplazamiento += largo_nombre_tabla;
		memcpy(buffer+desplazamiento, &tabla->consistencia, sizeof(criterio));
		desplazamiento += sizeof(criterio);
	}
	list_iterate(tablas, (void*)_cargar_en_buffer);
	return buffer;
}

t_list *deserializar_response_describe(t_prot_mensaje *mensaje, t_log *logger) {
	size_t tamanio_del_buffer = mensaje->tamanio_total - sizeof(t_header);
	int desplazamiento = 0;
	t_list *retorno = list_create();
	while(desplazamiento < tamanio_del_buffer) {
		t_response_describe *describe = malloc(sizeof(t_response_describe));
		int largo_nombre_tabla = 0;
		memcpy(&largo_nombre_tabla, mensaje->payload+desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);

		describe->tabla = malloc(largo_nombre_tabla+1);
		memset(describe->tabla, 0, largo_nombre_tabla+1);
		memcpy(describe->tabla, mensaje->payload+desplazamiento, largo_nombre_tabla);
		describe->tabla[largo_nombre_tabla] = '\0';
		desplazamiento += largo_nombre_tabla;

		criterio consistencia = INVALIDO;
		memcpy(&consistencia, mensaje->payload+desplazamiento, sizeof(criterio));
		desplazamiento += sizeof(criterio);

		describe->consistencia = consistencia;
		list_add(retorno, describe);
	}
	return retorno;
}

void imprimir_datos_describe(t_list *tablas) {
	void _mostrar_datos(t_response_describe *tabla) {
		printf("|\t %s\t|\t %s\t\t |\n", tabla->tabla, criterio_to_string(tabla->consistencia));
		printf("+------------------------------------------------+\n");
	}
	printf("+------------------------------------------------+\n");
	printf("|\t Tabla\t\t|\tConsistencia\t |\n");
	printf("+------------------------------------------------+\n");
	list_iterate(tablas, (void*)_mostrar_datos);
}


void intercambir_memorias_conectadas(t_list *mi_tabla, t_list *otra_tabla) {
	void _agregar_nuevos(t_memoria_conectada *memoria_en_otra_lista) {
		bool _desde(t_memoria_conectada *memoria_de_mi_Lista) {
			//printf("Memoria otra lista: %d\n", memoria_en_otra_lista->nombre);
			//printf("Memoria de mi lista: %d\n", memoria_de_mi_Lista->nombre);
			return memoria_en_otra_lista->nombre == memoria_de_mi_Lista->nombre;

		}
		if(!list_find(mi_tabla, (void *)_desde)) {
			//printf("Agrego memoria: %d", memoria_en_otra_lista->nombre);
			list_add(mi_tabla, memoria_en_otra_lista);
		} else {
			if(memoria_en_otra_lista == NULL) printf("\n\nITERA EN NULOS\n\n");
			free(memoria_en_otra_lista->ip);
			free(memoria_en_otra_lista);
		}
	}
	list_iterate(otra_tabla, (void *)_agregar_nuevos);
	list_destroy(otra_tabla);
}

void *serializar_tabla_gossip(size_t tamanio_del_buffer, t_list *tabla) {
	void *buffer = malloc(tamanio_del_buffer);
		memset(buffer, 0, tamanio_del_buffer);
		int desplazamiento = 0;
		void _cargar_en_buffer(t_memoria_conectada *memoria) {
			int largo_ip = strlen(memoria->ip);
			memcpy(buffer+desplazamiento, &memoria->nombre, sizeof(int));
			desplazamiento += sizeof(int);
			memcpy(buffer+desplazamiento, &largo_ip, sizeof(int));
			desplazamiento += sizeof(int);
			memcpy(buffer+desplazamiento, memoria->ip, largo_ip);
			desplazamiento += largo_ip;
			memcpy(buffer+desplazamiento, &memoria->puerto, sizeof(int));
			desplazamiento += sizeof(int);
		}
		list_iterate(tabla, (void*)_cargar_en_buffer);
		return buffer;
}

t_list *deserializar_tabla_gossip(t_prot_mensaje *mensaje, t_log *logger) {
	size_t tamanio_del_buffer = mensaje->tamanio_total - sizeof(t_header);
	int desplazamiento = 0;
	t_list *retorno = list_create();
	while(desplazamiento < tamanio_del_buffer) {
		t_memoria_conectada *memoria = malloc(sizeof(t_memoria_conectada));
		memcpy(&memoria->nombre, mensaje->payload+desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);

		int largo_ip = 0;
		memcpy(&largo_ip, mensaje->payload+desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);

		memoria->ip = malloc(largo_ip+1);
		memset(memoria->ip, 0, largo_ip+1);
		memcpy(memoria->ip, mensaje->payload+desplazamiento, largo_ip);
		memoria->ip[largo_ip] = '\0';
		desplazamiento += largo_ip;

		memcpy(&memoria->puerto, mensaje->payload+desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);

		list_add(retorno, memoria);
	}
	return retorno;
}

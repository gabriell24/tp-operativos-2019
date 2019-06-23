#include "manejo_memoria.h"

void conectar_a_memoria(char *ip, int puerto) {
	log_info(logger, "[Conexión] Esperando conectar a memoria");
	int socket_memoria = conectar_a_servidor(ip, puerto, KERNEL);

	char* handshake = "hola soy kernel, mucho gusto!";
	int largo_palabra = strlen(handshake); //No sumar caracteres por barra cero
	int numero_a_mandar = 991;
	size_t tamanio_buffer = sizeof(int)*2 + largo_palabra;
	void* buffer = malloc(tamanio_buffer);

	memset(buffer, 0, tamanio_buffer);
	memcpy(buffer, &numero_a_mandar, sizeof(int));
	memcpy(buffer+sizeof(int), &largo_palabra, sizeof(int));
	memcpy(buffer+sizeof(int)*2, handshake, largo_palabra);

	log_debug(logger, "[Conexión] El tamanio del buffer de handshake es: %d", tamanio_buffer);
	prot_enviar_mensaje(socket_memoria, ENVIO_DATOS, tamanio_buffer, buffer);
	free(buffer);
	t_prot_mensaje *respuesta_memoria = prot_recibir_mensaje(socket_memoria);

	t_memoria_conectada *memoria_conectada = malloc(sizeof(t_memoria_conectada));
	memset(&memoria_conectada->nombre, 0, sizeof(int));
	memcpy(&memoria_conectada->nombre, respuesta_memoria->payload, respuesta_memoria->tamanio_total-sizeof(t_header));
	memoria_conectada->ip = string_duplicate(ip);
	memoria_conectada->puerto = puerto;
	memcpy(&memoria_conectada->socket, &socket_memoria, sizeof(int));
	list_add(tabla_gossip, memoria_conectada);
	prot_destruir_mensaje(respuesta_memoria);

	log_info(logger, "[Conexión] Memoria conectada, hago un describe");
	kernel_describe(socket_memoria, "");
	int *ptr_socket = malloc(sizeof(int));
	*ptr_socket = socket_memoria;
	pthread_create(&hilo_manejo_memorias, NULL, (void*)recibir_mensajes_de_memoria, ptr_socket);
}

void recibir_mensajes_de_memoria(int *ptr_socket) {
	t_prot_mensaje *mensaje_de_memoria;
	bool cortar_while = false;
	bool intentar_reconectar = true;
	int socket_memoria = *ptr_socket;
	free(ptr_socket);
	while(!consola_ejecuto_exit && !cortar_while) {
		mensaje_de_memoria = prot_recibir_mensaje(socket_memoria);
		switch(mensaje_de_memoria->head) {
		case RESPUESTA_DESCRIBE: {
			log_info(logger, "Llegó el describe");
			t_list *describe_recibido = deserializar_response_describe(mensaje_de_memoria, logger);
			//list_add_all(describe_tablas, deserializar_response_describe(mensaje_de_memoria, logger));
			actualizar_describe(describe_recibido);
			list_destroy(describe_recibido);
			if(!describe_tablas) log_error(logger, "[Describe] Llego vació");
			imprimir_datos_describe(describe_tablas);
		} break;

		case FUNCION_SELECT: {
			size_t tamanio_respuesta_select = mensaje_de_memoria->tamanio_total-sizeof(t_header);
			char *respuesta_select = malloc(tamanio_respuesta_select+1);
			memset(respuesta_select, 0, tamanio_respuesta_select+1);
			memcpy(respuesta_select, mensaje_de_memoria->payload, tamanio_respuesta_select);
			respuesta_select[tamanio_respuesta_select] = '\0';
			log_info(logger, "[Respuesta Select] %s", respuesta_select);
			free(respuesta_select);
		} break;

		case DESCONEXION: {
			log_error(logger, "Se desconectó memoria");
			cortar_while = true;
		} break;

		default:
			cortar_while = true;
			log_error(logger, "Mensaje no conocido. Header: %d, corto el recv", mensaje_de_memoria->head);
		}
		prot_destruir_mensaje(mensaje_de_memoria);
		if(cortar_while) break;
	}
}

#include "manejo_memoria.h"

void conectar_a_memoria(char *ip, int puerto) {
	log_info(logger, "[Conexión] Esperando conectar a memoria");
	socket_memoria = conectar_a_servidor(ip, puerto, KERNEL);
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

	log_info(logger, "[Conexión] Memoria conectada, hago un describe");
	kernel_describe("");
	pthread_create(&hilo_observer_configs, NULL, (void*)recibir_mensajes_de_memoria, NULL);
}

void recibir_mensajes_de_memoria() {
	t_prot_mensaje *mensaje_de_memoria;
	bool cortar_while = false;
	bool intentar_reconectar = true;
	while(!consola_ejecuto_exit && !cortar_while) {
		mensaje_de_memoria = prot_recibir_mensaje(socket_memoria);
		switch(mensaje_de_memoria->head) {
		case RESPUESTA_DESCRIBE: {
			log_info(logger, "Llegó el describe");
			describe_tablas = deserializar_response_describe(mensaje_de_memoria, logger);
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
			log_error(logger, "Mensaje no conocido. %d", mensaje_de_memoria->head);
		}
		prot_destruir_mensaje(mensaje_de_memoria);
	}
}

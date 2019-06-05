#include "manejo_memoria.h"

void conectar_a_memoria() {
	log_info(logger, "[Conexión] Esperando conectar a memoria");
	socket_memoria = conectar_a_servidor(kernel_config.ip_memoria, kernel_config.puerto_memoria, KERNEL);
	log_info(logger, "[Conexión] Esperando conectar a memoria2");
	char* handshake = "hola soy kernel, mucho gusto!";
	//Sumo uno por el barra cero.
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
	kernel_describe(""); //########## OJO DEBE SER GLOBAL
	recibir_mensajes_de_memoria();
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

		case DESCONEXION: {
			log_error(logger, "Se desconectó memoria");
			cortar_while = true;
		} break;

		default:
			log_error(logger, "Mensaje no conocido. %d", mensaje_de_memoria->head);
		}
		prot_destruir_mensaje(mensaje_de_memoria);
	}
	if(cortar_while && intentar_reconectar) {
		conectar_a_memoria();
	}
}

#include "manejo_memoria.h"

void manejar_memorias() {
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

	log_info(logger, "[Conexión] Memoria conectada");
}

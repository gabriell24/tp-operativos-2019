#include "manejo_memoria.h"

void conectar_a_memoria(char *ip, int puerto) {
	loguear(info, logger, "[Conexión] Esperando conectar a memoria");
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

	loguear(debug, logger, "[Conexión] El tamanio del buffer de handshake es: %d", tamanio_buffer);
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
	prot_enviar_mensaje(socket_memoria, DAME_POOL_MEMORIAS, 0, NULL);

	loguear(info, logger, "[Conexión] Memoria conectada, hago un describe");
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
			loguear(info, logger, "Llegó el describe");
			t_list *describe_recibido = deserializar_response_describe(mensaje_de_memoria, logger);
			//list_add_all(describe_tablas, deserializar_response_describe(mensaje_de_memoria, logger));
			//TODO SINCRONIZAR EL _ADD EN DESCRIBE
			actualizar_describe(describe_recibido);
			list_destroy(describe_recibido);
			if(!describe_tablas) loguear(error, logger, "[Describe] Llego vació");
			imprimir_datos_describe(describe_tablas);
		} break;

		case RESPUESTA_POOL_MEMORIAS: {
			loguear(info, logger, "Llego el mensaje respuesta pool de memorias");
			t_list *tabla_gossip_recibida = deserializar_tabla_gossip(mensaje_de_memoria, logger);
			/*mostrar_tabla_gossip(tabla_gossip_recibida, logger);
			intercambir_memorias_conectadas(tabla_gossip, tabla_gossip_recibida);
			mostrar_tabla_gossip(tabla_gossip, logger);*/
			void _conectar_todas(t_memoria_conectada *memoria) {
				loguear(info, logger, "Conectando a %s:%d", memoria->ip, memoria->puerto);
				conectar_a_memoria(memoria->ip, memoria->puerto);
			}
			list_iterate(tabla_gossip_recibida, (void *)_conectar_todas);
			//list_destroy_and_destroy_elements(tabla_gossip_recibida, (void *)_limpiar_tabla_gossip); //TODO COMPLETAR
		} break;

		case FUNCION_SELECT: {
			size_t tamanio_respuesta_select = mensaje_de_memoria->tamanio_total-sizeof(t_header);
			char *respuesta_select = malloc(tamanio_respuesta_select+1);
			memset(respuesta_select, 0, tamanio_respuesta_select+1);
			memcpy(respuesta_select, mensaje_de_memoria->payload, tamanio_respuesta_select);
			respuesta_select[tamanio_respuesta_select] = '\0';
			loguear(info, logger, "[Respuesta Select] %s", respuesta_select);
			free(respuesta_select);
		} break;

		case RESPUESTA_CREATE: {
			size_t tamanio_respuesta_create = mensaje_de_memoria->tamanio_total-sizeof(t_header);
			char *respuesta_create = malloc(tamanio_respuesta_create+1);
			memset(respuesta_create, 0, tamanio_respuesta_create+1);
			memcpy(respuesta_create, mensaje_de_memoria->payload, tamanio_respuesta_create);
			respuesta_create[tamanio_respuesta_create] = '\0';
			loguear(info, logger, "[Respuesta Create] %s", respuesta_create);
			if(string_starts_with(respuesta_create, "[Create] Tabla creada ")) {
				//TODO SINCRONIZAR EL _ADD EN DESCRIBE
				/*char *tabla = string_substring_from(respuesta_create, strlen("[Create] Tabla creada "));*/
				t_response_describe *agregar = malloc(sizeof(t_response_describe));
				agregar->tabla = string_duplicate(ultimo_create_enviado->tabla);
				agregar->consistencia = ultimo_create_enviado->consistencia;
				list_add(describe_tablas, agregar);
			}
			free(ultimo_create_enviado->tabla);
			free(ultimo_create_enviado);
			free(respuesta_create);
			pthread_mutex_unlock(&mutex_create);
		} break;

		case RESPUESTA_DROP: {
			size_t tamanio_respuesta_drop = mensaje_de_memoria->tamanio_total-sizeof(t_header);
			char *respuesta_drop = malloc(tamanio_respuesta_drop+1);
			memset(respuesta_drop, 0, tamanio_respuesta_drop+1);
			memcpy(respuesta_drop, mensaje_de_memoria->payload, tamanio_respuesta_drop);
			respuesta_drop[tamanio_respuesta_drop] = '\0';
			loguear(info, logger, "[DROP] Se eliminó: %s", respuesta_drop);
			bool _eliminar_por_nombre(t_response_describe *metadata) {
				return string_equals_ignore_case(metadata->tabla, respuesta_drop);
			}
			list_remove_by_condition(describe_tablas, (void *)_eliminar_por_nombre);
			free(respuesta_drop);
		} break;

		case DESCONEXION: {
			loguear(error, logger, "Se desconectó memoria");
			quitar_memoria_de_criterio(socket_memoria);
			limpiar_conectado_en_gossip(socket_memoria);
			cortar_while = true;
		} break;

		default:
			cortar_while = true;
			loguear(error, logger, "Mensaje no conocido. Header: %d, corto el recv", mensaje_de_memoria->head);
		}
		prot_destruir_mensaje(mensaje_de_memoria);
		if(cortar_while) break;
	}
}

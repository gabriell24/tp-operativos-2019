#include "memoria.h"

int main() {
	//Cómo el observer nunca se detiene, uso una variable global para avisarle
	consola_ejecuto_exit = false;
	levantar_archivo_configuracion();
	logger = log_create("memoria.log","MEMORIA", true,
			memoria_config.en_produccion ? LOG_LEVEL_INFO : LOG_LEVEL_DEBUG);

	/*
	 * Invierto el orden, para que si no puede iniciar la memoria
	 * segun start up del tp, tampoco pueda recibir conexiones
	 */
	socket_lissandra = conectar_a_servidor(memoria_config.ip_lissandra, memoria_config.puerto_lissandra, MEMORIA);
	tamanio_value = recibir_datos_de_fs(socket_lissandra);
	iniciar_memoria();

	socket_servidor = levantar_servidor(memoria_config.puerto_escucha);

	log_info(logger, "Memoria %d iniciado", memoria_config.numero_memoria);
	printear_configuraciones();

	//<<1- inotify
	int fd_inotify = inotify_init();
	if (fd_inotify < 0) {
		perror("No puedo obtener fd para inotify");
	}
	int watch_descriptor = inotify_add_watch(fd_inotify, ".", IN_MODIFY);
	int *ptr_fd_inotify = malloc(sizeof(int*));
	*ptr_fd_inotify = fd_inotify;
	pthread_create(&hilo_observer_configs,NULL, (void*)escuchar_cambios_en_configuraciones, (void*)ptr_fd_inotify);
	//1>>
	pthread_create(&hilo_consola, NULL, (void*)consola, NULL);

	int *ptr_socket_servidor = malloc(sizeof(int));
	*ptr_socket_servidor = socket_servidor;
	pthread_create(&hilo_aceptar_clientes,NULL, (void*)aceptar_clientes, ptr_socket_servidor);

	pthread_join(hilo_consola, NULL);
	log_info(logger, "[Memoria] Proceso finalizado.");
	pthread_join(hilo_observer_configs, NULL);

	inotify_rm_watch(fd_inotify, watch_descriptor);
	close(fd_inotify);
	free(ptr_fd_inotify);

	//Limpiar cada estructura
	free(memoria);
	log_destroy(logger);
}

int recibir_datos_de_fs(int socket) {
	t_prot_mensaje *mensaje = prot_recibir_mensaje(socket);
	int value;
	memcpy(&value, mensaje->payload, sizeof(int));
	log_info(logger, "[Tamanio] Value = %d", value);
	prot_destruir_mensaje(mensaje);
	return value;
}

/* Funcióm creada para verificar que recargue las variables luego de que inotify
 * dispare el evento onChange
 */
void printear_configuraciones() {
	log_debug(logger, "[Configuración] Puerto: %d", memoria_config.puerto_escucha);
	log_debug(logger, "[Configuración] Retardo acceso a memoria principal: %d", memoria_config.retardo_accesso_a_mp);
	log_debug(logger, "[Configuración] Retardo acceso a lissandra: %d", memoria_config.retardo_accesso_a_fs);
	log_debug(logger, "[Configuración] Timpo entre journaling: %d", memoria_config.tiempo_journaling);
	log_debug(logger, "[Configuración] Tiempo entre gossiping: %d", memoria_config.tiempo_gossiping);
}

void escuchar_cambios_en_configuraciones(void *ptr_fd) {
	int file_descriptor = *((int *) ptr_fd);

	while(!consola_ejecuto_exit) {
	    char buffer[BUF_LEN];
	    struct inotify_event *event = NULL;

	    int len = read(file_descriptor, buffer, sizeof(buffer));
	    if (len < 0) {
	    	perror("inotify no puede leer el fd");
	    }

	    event = (struct inotify_event *) buffer;
	    while(event != NULL) {
	    	if(event->len > 0) {
				if((event->mask & IN_MODIFY) &&
					string_equals_ignore_case("memoria.config", event->name)) {

						log_info(logger, "[Info] Se modificó memoria.config");
						//Esto no necesariamente debe ser así, sería mejor solo que setee lo que necesite.
						log_debug(logger, "[DEBUG] pre releer configs");
						levantar_archivo_configuracion();
						log_debug(logger, "[DEBUG] termino de leer configs");
						printear_configuraciones();

				}
	    	}

	        len -= sizeof(*event) + event->len;
	        if (len > 0)
	            event = ((void *) event) + sizeof(event) + event->len;
	        else
	            event = NULL;
	    }

	}

}

void aceptar_clientes(int *ptr_socket_servidor) {
	int socket_servidor = *ptr_socket_servidor;
	free(ptr_socket_servidor);
	int socket_cliente;
	struct sockaddr_in direccion_cliente;
	unsigned int tamanio_direccion = sizeof(direccion_cliente);

	while((!consola_ejecuto_exit) && (socket_cliente = accept(socket_servidor, (void*) &direccion_cliente, &tamanio_direccion)) > 0) {
		t_prot_mensaje* mensaje_del_cliente = prot_recibir_mensaje(socket_cliente);
		t_cliente cliente_recibido = *((t_cliente*) mensaje_del_cliente->payload);
		switch(cliente_recibido) {
		case KERNEL: {
			log_debug(logger, "[Conexión] Viene del kernel");
			pthread_t recibir_mensajes_de_kernel;
			int* socket_kernel = (int*) malloc (sizeof(int));
			*socket_kernel = socket_cliente;
			pthread_create(&recibir_mensajes_de_kernel,NULL, (void*)escuchar_kernel, socket_kernel);
		} break;
		case MEMORIA: {
			log_info(logger, "[Conexión] Memoria conectada");
		} break;
		default:
			log_error(logger, "[Conexión] Cliente desconocido");
		}
		prot_destruir_mensaje(mensaje_del_cliente);
	}
}

/* Esta es la parte pasiva del intercambio de gossiping. */
void recibir_datos_gossiping() {


}

void escuchar_kernel(int *socket_origen) {
	int socket_kernel = *socket_origen;
	free(socket_origen);
	bool cortar_while = false;
	//t_prot_mensaje *mensaje_de_kernel;
	while (!consola_ejecuto_exit && !cortar_while) {
		t_prot_mensaje *mensaje_de_kernel = prot_recibir_mensaje(socket_kernel);
		switch(mensaje_de_kernel->head) {
			case DESCONEXION: {
				log_warning(logger, "[Desconexión] Mato el hilo, ya no podrá recibir mensajes");
				cortar_while = true;
			} break;

			case ENVIO_DATOS: {
				log_info(logger, "[Conexión] Kernel conectado");
				int tamanio_buffer = mensaje_de_kernel->tamanio_total - sizeof(t_header);
				int numero;
				int largo_de_handshake;
				memcpy(&numero, mensaje_de_kernel->payload, sizeof(int));
				memcpy(&largo_de_handshake, mensaje_de_kernel->payload+sizeof(int), sizeof(int));
				char handshake[largo_de_handshake+1];
				memcpy(handshake, mensaje_de_kernel->payload + sizeof(int)*2, largo_de_handshake);
				log_info(logger, "[Conexión] Saludo: %s, Número: %d", handshake, numero);
			} break;

			case FUNCION_SELECT: {
				log_debug(logger, "[Conexión] pre deserializar request select");
				t_request_select *buffer = deserializar_request_select(mensaje_de_kernel);
				log_info(logger, "Hacer select con [TABLA = %s, KEY = %d]", buffer->tabla, buffer->key);
				size_t tamanio_del_buffer = sizeof(int) + strlen(buffer->tabla) + sizeof(uint16_t);
				void *buffer_serializado = serializar_request_select(buffer->tabla, buffer->key);
				prot_enviar_mensaje(socket_lissandra, FUNCION_SELECT, tamanio_del_buffer, buffer_serializado);
				free(buffer_serializado);
				t_prot_mensaje *mensaje_de_lissandra = prot_recibir_mensaje(socket_lissandra);
				if(mensaje_de_lissandra->head == REGISTRO_TABLA) {
					log_info(logger, "LLego el dato de fs");
				}
				prot_destruir_mensaje(mensaje_de_lissandra);
				//free(buffer->tabla);
				//free(buffer->key);
				//free(buffer);
			} break;

			case FUNCION_INSERT: {
				log_debug(logger, "[Conexión] pre deserializar request insert");
				t_request_insert *biffer = deserializar_request_insert(mensaje_de_kernel);
				log_info(logger, "Hacer insert con [TABLA = %s, KEY = %d, VALUE = %s, EPOCH = %d]", biffer->nombre_tabla, biffer->key, biffer->value, biffer->epoch);
				free(biffer->nombre_tabla);
				free(biffer->value);
				free(biffer);
			} break;

			case FUNCION_CREATE: {
				log_debug(logger, "[Conexión] pre deserializar resquest CREATE");
				t_request_create *buffer = deserializar_request_create(mensaje_de_kernel);
				log_info(logger, "Hacer create con [NombreTabla = %s, tipoConsistencia = %s, numeroPart = %d, compatTime = %d]", buffer->nombre_tabla, buffer->tipo_consistencia, buffer->numero_particiones, buffer->compaction_time);
				//free(buffer->tabla);
				//free(buffer->key);
				//free(buffer);
			} break;
			case FUNCION_DESCRIBE: {
				log_debug(logger, "[Conexión] pre deserializar resquest DESCRIBE");
				char* tamanio = mensaje_de_kernel->payload;
				int tam = mensaje_de_kernel->tamanio_total-sizeof(mensaje_de_kernel->head);
				if(tam == 0){
					printf("Describe nulo\n");
				}
				else{
					char* tabla = malloc(tam+1);
					memset(tabla, 0, tam+1);
					memcpy(tabla, mensaje_de_kernel->payload,tam);
					tabla[tam] = '\0';
					printf("Describe con tabla: %s\n",tabla);
					free(tabla);
				}
			} break;

			case FUNCION_DROP: {
				int tab = mensaje_de_kernel->tamanio_total-sizeof(mensaje_de_kernel->head);
				char* tabla = malloc(tab+1);
				memset(tabla, 0, tab+1 );
				memcpy(tabla, mensaje_de_kernel->payload,tab);
				tabla[tab] = '\0';
				printf("Drop con tabla: %s\n",tabla);
				free(tabla);

			} break;

			default: {
				cortar_while = true;
				log_warning(logger, "Me llegó un mensaje desconocido");
				break;
			}
		}
		prot_destruir_mensaje(mensaje_de_kernel);
	}
}


void iniciar_memoria() {
	memoria = malloc(memoria_config.tamanio_de_memoria);
	log_debug(logger, "Inicio de memoria: [Hex]=%p", memoria);
	size_t tamanio_key = sizeof(uint16_t);
	size_t tamanio_timestamp = sizeof(int);
	log_debug(logger, "SizeOf Key: %d, SizeOf Timestamp:%d", tamanio_key, tamanio_timestamp);
	tamanio_de_pagina = tamanio_value + tamanio_key + tamanio_timestamp;
	tdp = list_create();
	tds = list_create();
	int total_de_frames = memoria_config.tamanio_de_memoria / tamanio_de_pagina;
	log_debug(logger, "Tamanio de memoria: %d, Tamanio de página: %d, Total de frames: %d", memoria_config.tamanio_de_memoria, tamanio_de_pagina, total_de_frames);
	int i = 1;
	for(;i <= total_de_frames; i++) {
		t_est_tdp pagina;
		pagina.modificado = 0;
		pagina.nro_pag = i;
		pagina.ptr_posicion = memoria + ((i-1) * tamanio_de_pagina);
		log_debug(logger, "Posicion de memoria pag %d: [Hex]=%p", i, pagina.ptr_posicion);
		list_add(tdp, &pagina);
	}


}

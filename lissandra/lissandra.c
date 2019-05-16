#include "lissandra.h"

int main() {
	//Variable global para avisarle al hilo del observer que frene.
	consola_ejecuto_exit = false;
	levantar_archivo_configuracion();
	logger = log_create("lissandra.log","LISSANDRA", true,
			fs_config.en_produccion ? LOG_LEVEL_INFO : LOG_LEVEL_DEBUG);
	iniciar_fs(fs_config.punto_montaje);

	socket_servidor = levantar_servidor(fs_config.puerto_escucha);
	log_info(logger, "Lissandra iniciado");
	printear_configuraciones();

	t_list_memtable = list_create();
	cargar_datos_fake();

	//<<1- inotify
	int fd_inotify = inotify_init();
	if (fd_inotify < 0) {
		perror("No puedo obtener fd para inotify");
	}
	int watch_descriptor = inotify_add_watch(fd_inotify, ".", IN_MODIFY);
	int *ptr_fd_inotify = malloc(sizeof(int*));
	*ptr_fd_inotify = fd_inotify;
	pthread_create(&hilo_observer_configs, NULL, (void*)escuchar_cambios_en_configuraciones, (void*)ptr_fd_inotify);
	//1>>
	int *ptr_socket_server = malloc(sizeof(int));
	*ptr_socket_server = socket_servidor;
	pthread_create(&hilo_conexion_memoria, NULL, (void*)aceptar_conexion_de_memoria, ptr_socket_server);
	pthread_create(&hilo_consola, NULL, (void*)consola, NULL);
	pthread_join(hilo_consola, NULL);
	log_info(logger, "[Lissandra] Proceso finalizado.");
	//pthread_join(hilo_conexion_memoria, NULL);
	pthread_join(hilo_observer_configs, NULL);

	inotify_rm_watch(fd_inotify, watch_descriptor);
	close(fd_inotify);
	free(ptr_fd_inotify);
	log_destroy(logger);
}

/* Funcióm creada para verificar que recargue las variables luego de que inotify
 * dispare el evento onChange
 */
void printear_configuraciones() {
	log_debug(logger, "[Configuración] Iniciando en %s", fs_config.punto_montaje);
	log_debug(logger, "[Configuración] Puerto: %d", fs_config.puerto_escucha);
	log_debug(logger, "[Configuración] Retardo: %d", fs_config.retardo_ms);
	log_debug(logger, "[Configuración] Tiempo para dumpeo: %d", fs_config.tiempo_dump_ms);
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
					string_equals_ignore_case("lissandra.config", event->name)) {

						log_info(logger, "[Info] Se modificó lissandra.config");
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

void cargar_datos_fake() {
	printf("##################### INICIO TESTS ###################");
	char *tabla = "miTabla";
	uint16_t key = 15;
	int timestamp = 1556415179;
	char *value = "unValue"; //Notar que no se libera por no reservar con malloc

	t_registro unRegistro;
	unRegistro.key = key;
	unRegistro.timestamp = timestamp;
	unRegistro.value = value;

	t_list *registros = list_create();
	list_add(registros, &unRegistro);

	t_memtable unaTabla;
	unaTabla.tabla = tabla;
	unaTabla.t_registro = registros;

	printf("Key value: %hu\n", key);
	printf("Key en memtable: %hu\n", ((t_registro*)list_get(unaTabla.t_registro, 0))->key);

	FILE* fd;
	char *pathfinal = strdup(fs_config.punto_montaje);
	string_append_with_format(&pathfinal, "Tables/%s/%s", tabla, "fake.txt");
	printf("Path a buscar: %s\n", pathfinal);
	fd = fopen(pathfinal, "a");
	fwrite(((t_registro*)list_get(unaTabla.t_registro, 0))->value, strlen(value), 1, fd);
	fwrite("\n", 1, 1, fd);
	fclose(fd);

	/*libero memoria:
	 * Limpio los datos en medida de no perder referencia,
	 * es decir, no destruyo una lista, sin antes liberar
	 * los datos en el heap que haya guardado
	 */

	list_destroy(registros);
	free(pathfinal);
	printf("##################### FIN TESTS ###################\n");

}

void aceptar_conexion_de_memoria(int *ptr_socket_servidor) {
	int socket_servidor = *ptr_socket_servidor;
	free(ptr_socket_servidor);
	int socket_cliente;
	struct sockaddr_in direccion_cliente;
	unsigned int tamanio_direccion = sizeof(direccion_cliente);

	while(!consola_ejecuto_exit) {
		socket_cliente = accept(socket_servidor, (void*) &direccion_cliente, &tamanio_direccion);
		if(!(socket_cliente > 0)) {
			perror("No pude aceptar a memoria: ");
			break;
		}
		else {
			t_prot_mensaje* mensaje_del_cliente = prot_recibir_mensaje(socket_cliente);
			t_cliente cliente_recibido = *((t_cliente*) mensaje_del_cliente->payload);
			switch(cliente_recibido) {

			case MEMORIA: {
				log_info(logger, "[Conexión] Memoria conectada. Enviando tamanio del value");

				//Envio el tamanio del value
				size_t tamanio_buffer = sizeof(int);
				void *buffer = malloc(tamanio_buffer);
				memset(buffer, 0, sizeof(int));
				memcpy(buffer, &fs_config.tamanio_value, sizeof(int));
				prot_enviar_mensaje(socket_cliente, ENVIO_DATOS, tamanio_buffer, buffer);
				//Fin envio del tamanio del value

				pthread_t recibir_mensajes_de_memoria;
				int* socket_kernel = (int*) malloc (sizeof(int));
				*socket_kernel = socket_cliente;
				pthread_create(&recibir_mensajes_de_memoria, NULL, (void*)escuchar_memoria, socket_kernel);
			} break;
			default:
				log_error(logger, "[Conexión] Cliente desconocido");
			}
			prot_destruir_mensaje(mensaje_del_cliente);
		}
	}
}

void escuchar_memoria(int *ptr_socket_cliente) {
	int socket_memoria = *ptr_socket_cliente;
	free(ptr_socket_cliente);
	bool cortar_while = false;
	//t_prot_mensaje *mensaje_de_kernel;
	while (!consola_ejecuto_exit && !cortar_while) {
		t_prot_mensaje *mensaje_de_memoria = prot_recibir_mensaje(socket_memoria);
		switch(mensaje_de_memoria->head) {
			case DESCONEXION: {
				log_warning(logger, "[Desconexión] Mato el hilo, ya no podrá recibir mensajes");
				cortar_while = true;
			} break;

			case FUNCION_SELECT: {
				t_request_select *buffer = deserializar_request_select(mensaje_de_memoria);
				log_info(logger, "[Select] Tabla: %s", buffer->tabla);
			} break;

			default: {
				cortar_while = true;
				log_warning(logger, "Me llegó un mensaje desconocido %d", mensaje_de_memoria->head);
				break;
			}

		}
		prot_destruir_mensaje(mensaje_de_memoria);
	}
}


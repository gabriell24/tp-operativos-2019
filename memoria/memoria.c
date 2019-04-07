#include "memoria.h"

int main() {
	//Cómo el observer nunca se detiene, uso una variable global para avisarle
	finalizar_proceso_normal = false;
	levantar_archivo_configuracion();
	logger = log_create("memoria.log","MEMORIA", true,
			memoria_config.en_produccion ? LOG_LEVEL_INFO : LOG_LEVEL_DEBUG);

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

	atender_memoria(socket_servidor);
	pthread_join(hilo_consola, NULL);
	//log_info(logger, "Finalizó la consola, debería morir el proceso");
	pthread_join(hilo_observer_configs, NULL);

	inotify_rm_watch(fd_inotify, watch_descriptor);
	close(fd_inotify);
	free(ptr_fd_inotify);
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

	while(!finalizar_proceso_normal) {
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

void atender_memoria(int socket_servidor) {
	int socket_memoria;
	struct sockaddr_in direccion_cliente;
	unsigned int tamanio_direccion = sizeof(direccion_cliente);
	//Burocracia...

	//Se aceptan clientes cuando los haya
	// accept es una funcion bloqueante, si no hay ningun cliente esperando ser atendido, se queda esperando a que venga uno.
	log_debug(logger, "[Conexión] Esperando conexión de kernel");
	socket_memoria = accept(socket_servidor, (void*) &direccion_cliente, &tamanio_direccion);
	if(socket_memoria < 0) {
		perror("no se pudo aceptar conexión.");
	}
	while(!finalizar_proceso_normal) {

		t_prot_mensaje* mensaje_de_memoria = prot_recibir_mensaje(socket_memoria);
		if(mensaje_de_memoria->head == CONEXION) {
			log_info(logger, "[Conexión] Kernel conectado");
			int tamanio_buffer = mensaje_de_memoria->tamanio_total - sizeof(t_header);
			void *path_recibido = malloc(tamanio_buffer);
			int numero;
			int largo_de_handshake;

			numero = *((int*)mensaje_de_memoria->payload);
			largo_de_handshake= *((int*)mensaje_de_memoria->payload + sizeof(int));
			char handshake[largo_de_handshake];
			memcpy(handshake, mensaje_de_memoria->payload + sizeof(int)*2, largo_de_handshake);
			log_info(logger, "[Conexión] Saludo: %s, Número: %d", handshake, numero);
		}
	}
}

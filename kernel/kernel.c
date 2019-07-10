#include "kernel.h"

int main() {
	//Cómo el observer nunca se detiene, uso una variable global para avisarle
	consola_ejecuto_exit = false;
	levantar_archivo_configuracion();
	tabla_gossip = list_create();
	describe_tablas = list_create();
	logger = log_create("kernel.log","KERNEL", true,
			kernel_config.en_produccion ? LOG_LEVEL_INFO : LOG_LEVEL_DEBUG);

	//log_info(logger, "Kernel iniciado");
	loguear(info, logger, "Kernel iniciado");
	pthread_mutex_init(&mutex_create, NULL);
	printear_configuraciones();
	iniciar_listas_de_criterios();

	//<<1- inotify
	int fd_inotify = inotify_init();
	if (fd_inotify < 0) {
		perror("No puedo obtener fd para inotify");
	}
	int watch_descriptor = inotify_add_watch(fd_inotify, ".", IN_MODIFY);
	int *ptr_fd_inotify = malloc(sizeof(int*));
	*ptr_fd_inotify = fd_inotify;

	conectar_a_memoria(kernel_config.ip_memoria, kernel_config.puerto_memoria);
	//while(!socket_memoria);
	pthread_create(&hilo_planificacion, NULL, (void*)iniciar_listas_planificacion, NULL);
	pthread_create(&hilo_observer_configs,NULL, (void*)escuchar_cambios_en_configuraciones, (void*)ptr_fd_inotify);
	//1>>
	pthread_create(&hilo_metadata_refresh, NULL, (void *)metadata_refresh, NULL);
	pthread_create(&hilo_consola, NULL, (void*)consola, NULL);
	pthread_join(hilo_consola, NULL);
	//log_info(logger, "[Kernel] Proceso finalizado.");
	loguear(info, logger, "[Kernel] Proceso finalizado.");
	pthread_join(hilo_observer_configs, NULL);

	inotify_rm_watch(fd_inotify, watch_descriptor);
	close(fd_inotify);
	free(ptr_fd_inotify);
	log_destroy(logger);
	limpiar_listas();
	pthread_mutex_destroy(&mutex_create);
	sem_destroy(&lqls_en_ready);
	sem_destroy(&instancias_exec);
	list_destroy_and_destroy_elements(describe_tablas, (void *)limpiar_tabla_describes);
	return 0;
}

/* Funcióm creada para verificar que recargue las variables luego de que inotify
 * dispare el evento onChange
 */
void printear_configuraciones() {
	/*
	 *  = config_get_int_value(configuracion, "QUANTUM");
	 = config_get_int_value(configuracion, "MULTIPROCESAMIENTO");
	 = config_get_int_value(configuracion, "REFRESCAR_METADATA");

	 */
	/*log_debug(logger, "[Configuración] Quantum en %d", kernel_config.quantum);
	log_debug(logger, "[Configuración] Nivel de multiprocesamiento: %d", kernel_config.multiprocesamiento);
	log_debug(logger, "[Configuración] Refrescar metadata cada: %d milisegundos", kernel_config.refrescar_metadata);
	log_debug(logger, "[Configuración] Retardo ciclo de ejecución: %d", kernel_config.retardo_ciclo_ejecucion);*/
	loguear(debug, logger, "[Configuración] Quantum en %d", kernel_config.quantum);
	loguear(debug, logger, "[Configuración] Nivel de multiprocesamiento: %d", kernel_config.multiprocesamiento);
	loguear(debug, logger, "[Configuración] Refrescar metadata cada: %d milisegundos", kernel_config.refrescar_metadata);
	loguear(debug, logger, "[Configuración] Retardo ciclo de ejecución: %d", kernel_config.retardo_ciclo_ejecucion);
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
					string_equals_ignore_case("kernel.config", event->name)) {

						//log_info(logger, "[Info] Se modificó kernel.config");
						loguear(info, logger, "[Info] Se modificó kernel.config");
						//log_debug(logger, "[DEBUG] pre releer configs");
						loguear(debug, logger, "[DEBUG] pre releer configs");
						recargar_archivo_configuracion();
						//log_debug(logger, "[DEBUG] termino de leer configs");
						loguear(debug, logger, "[DEBUG] termino de leer configs");
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


void actualizar_describe(t_list *nuevos) {
	void _buscar_y_agregar(t_response_describe *nuevo) {
		bool _ya_existe(t_response_describe *registro) {
			return string_equals_ignore_case(registro->tabla, nuevo->tabla);
		}
		if(!list_find(describe_tablas, (void *)_ya_existe)) {
			list_add(describe_tablas, nuevo);
		}
		else {
			free(nuevo->tabla);
			free(nuevo);
		}
	}
	list_iterate(nuevos, (void *)_buscar_y_agregar);
}

void limpiar_tabla_describes(t_response_describe *registro) {
	free(registro->tabla);
	free(registro);
}


void metadata_refresh() {
	while(!consola_ejecuto_exit) {
		loguear(info, logger, "Refrescando metadata");

		int memoria_destino = get_memoria(NULL);
		if(memoria_destino == -1) {
			loguear(error, logger, "No hay memoria conectada");
		} else {
			kernel_describe(memoria_destino, NULL);
		}
		usleep(kernel_config.refrescar_metadata * 1000);
	}
	pthread_detach(pthread_self());
}

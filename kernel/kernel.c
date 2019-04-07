#include "kernel.h"

int main() {
	//Cómo el observer nunca se detiene, uso una variable global para avisarle
	finalizar_proceso_normal = false;
	levantar_archivo_configuracion();
	logger = log_create("kernel.log","KERNEL", true,
			kernel_config.en_produccion ? LOG_LEVEL_INFO : LOG_LEVEL_DEBUG);

	log_info(logger, "Kernel iniciado");
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

	//Lo dejo comentado, porque entiendo que el flujo lo dispara el kernel cuando tenga algo para ejecutar
	pthread_create(&hilo_manejo_memorias, NULL, (void*)manejar_memorias, NULL);

	pthread_join(hilo_manejo_memorias, NULL);

	pthread_join(hilo_consola, NULL);
	log_info(logger, "Finalizó la consola, debería morir todo");
	pthread_join(hilo_observer_configs, NULL);

	inotify_rm_watch(fd_inotify, watch_descriptor);
	close(fd_inotify);
	free(ptr_fd_inotify);
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
	log_debug(logger, "[Configuración] Quantum en %d", kernel_config.quantum);
	log_debug(logger, "[Configuración] Nivel de multiprocesamiento: %d", kernel_config.multiprocesamiento);
	log_debug(logger, "[Configuración] Refrescar metadata cada: %d milisegundos", kernel_config.refrescar_metadata);
	log_debug(logger, "[Configuración] Retardo ciclo de ejecución: %d", kernel_config.retardo_ciclo_ejecucion);
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
					string_equals_ignore_case("kernel.config", event->name)) {

						log_info(logger, "[Info] Se modificó kernel.config");
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
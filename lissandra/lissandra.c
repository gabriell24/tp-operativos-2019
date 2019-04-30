#include "lissandra.h"

int main() {
	//Variable global para avisarle al hilo del observer que frene.
	consola_ejecuto_exit = false;
	levantar_archivo_configuracion();
	logger = log_create("lissandra.log","LISSANDRA", true,
			fs_config.en_produccion ? LOG_LEVEL_INFO : LOG_LEVEL_DEBUG);

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
	pthread_create(&hilo_observer_configs,NULL, (void*)escuchar_cambios_en_configuraciones, (void*)ptr_fd_inotify);
	//1>>
	pthread_create(&hilo_consola, NULL, (void*)consola, NULL);

	pthread_join(hilo_consola, NULL);
	log_info(logger, "[Lissandra] Proceso finalizado.");
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

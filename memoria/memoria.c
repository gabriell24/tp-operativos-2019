#include "memoria.h"

int main() {
	//Cómo el observer nunca se detiene, uso una variable global para avisarle
	consola_ejecuto_exit = false;
	tabla_gossip = list_create();
	levantar_archivo_configuracion();
	logger = log_create("memoria.log","MEMORIA", true,
			memoria_config.en_produccion ? LOG_LEVEL_INFO : LOG_LEVEL_DEBUG);

	pthread_mutex_init(&mutex_journaling, NULL);
	/*
	 * Invierto el orden, para que si no puede iniciar la memoria
	 * segun start up del tp, tampoco pueda recibir conexiones
	 */
	socket_lissandra = conectar_a_servidor(memoria_config.ip_lissandra, memoria_config.puerto_lissandra, MEMORIA);
	tamanio_value = recibir_datos_de_fs(socket_lissandra);
	iniciar_memoria();

	socket_servidor = levantar_servidor(memoria_config.puerto_escucha);
	//pthread_create(&hilo_gossip, NULL, (void *)iniciar_gossip, NULL);
	loguear(info, logger, "Memoria %d iniciado", memoria_config.numero_memoria);
	cargar_tabla_gossip();
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
	pthread_create(&hilo_journal, NULL, (void *)journal_automatico, NULL);
	pthread_create(&hilo_consola, NULL, (void*)consola, NULL);

	int *ptr_socket_servidor = malloc(sizeof(int));
	*ptr_socket_servidor = socket_servidor;
	pthread_create(&hilo_aceptar_clientes,NULL, (void*)aceptar_clientes, ptr_socket_servidor);

	pthread_join(hilo_consola, NULL);
	pthread_mutex_destroy(&mutex_journaling);
	loguear(info, logger, "[Memoria] Proceso finalizado.");
	pthread_join(hilo_observer_configs, NULL);

	inotify_rm_watch(fd_inotify, watch_descriptor);
	close(fd_inotify);
	free(ptr_fd_inotify);

	liberar_tabla_gossip(tabla_gossip);
	limpiar_memoria();
	limpiar_configuraciones();
	log_destroy(logger);
}

int recibir_datos_de_fs(int socket) {
	t_prot_mensaje *mensaje = prot_recibir_mensaje(socket);
	int value;
	memcpy(&value, mensaje->payload, sizeof(int));
	loguear(info, logger, "[Tamanio] Value = %d", value);
	prot_destruir_mensaje(mensaje);
	return value;
}

/* Funcióm creada para verificar que recargue las variables luego de que inotify
 * dispare el evento onChange
 */
void printear_configuraciones() {
	loguear(debug, logger, "[Configuración] Puerto: %d", memoria_config.puerto_escucha);
	loguear(debug, logger, "[Configuración] Retardo acceso a memoria principal: %d", memoria_config.retardo_accesso_a_mp);
	loguear(debug, logger, "[Configuración] Retardo acceso a lissandra: %d", memoria_config.retardo_accesso_a_fs);
	loguear(debug, logger, "[Configuración] Timpo entre journaling: %d", memoria_config.tiempo_journaling);
	loguear(debug, logger, "[Configuración] Tiempo entre gossiping: %d", memoria_config.tiempo_gossiping);
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

						loguear(info, logger, "[Info] Se modificó memoria.config");
						//Esto no necesariamente debe ser así, sería mejor solo que setee lo que necesite.
						loguear(debug, logger, "[DEBUG] pre releer configs");
						levantar_archivo_configuracion();
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
			loguear(debug, logger, "[Conexión] Viene del kernel");
			pthread_t recibir_mensajes_de_kernel;
			int* socket_kernel = (int*) malloc (sizeof(int));
			*socket_kernel = socket_cliente;
			pthread_create(&recibir_mensajes_de_kernel,NULL, (void*)escuchar_kernel, socket_kernel);
		} break;
		case MEMORIA: {
			loguear(info, logger, "[Conexión] Memoria conectada");
			pthread_t recibir_mensajes_de_memoria;
			int* socket_memoria = (int*) malloc (sizeof(int));
			*socket_memoria = socket_cliente;
			pthread_create(&recibir_mensajes_de_memoria, NULL, (void*)escuchar_memoria, socket_memoria);
		} break;
		default:
			loguear(error, logger, "[Conexión] Cliente desconocido");
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
				loguear(warning, logger, "[Desconexión] Mato el hilo, ya no podrá recibir mensajes");
				cortar_while = true;
			} break;

			case ENVIO_DATOS: {
				loguear(info, logger, "[Conexión] Kernel conectado");
				int *mi_nombre = malloc(sizeof(int));
				*mi_nombre = memoria_config.numero_memoria;
				prot_enviar_mensaje(socket_kernel, ENVIO_DATOS, sizeof(int), mi_nombre);
				free(mi_nombre);

				int numero = 0;
				int largo_de_handshake = 0;
				memcpy(&numero, mensaje_de_kernel->payload, sizeof(int));
				memcpy(&largo_de_handshake, mensaje_de_kernel->payload+sizeof(int), sizeof(int));
				char *handshake = malloc(largo_de_handshake+1);
				memset(handshake, 0, largo_de_handshake);
				memcpy(handshake, mensaje_de_kernel->payload + sizeof(int)*2, largo_de_handshake);
				handshake[largo_de_handshake] = '\0';
				loguear(info, logger, "[Conexión] Saludo: %s, Número: %d", handshake, numero);
				free(handshake);
			} break;

			case DAME_POOL_MEMORIAS: {
				loguear(info, logger, "llego dame pool");
				size_t tamanio_del_buffer = 0;
				void _calcular_buffer(t_memoria_conectada *memoria) {
					tamanio_del_buffer += sizeof(int)*3 + strlen(memoria->ip);
				}
				list_iterate(tabla_gossip, (void*)_calcular_buffer);
				void *buffer = serializar_tabla_gossip(tamanio_del_buffer, tabla_gossip);
				prot_enviar_mensaje(socket_kernel, RESPUESTA_POOL_MEMORIAS, tamanio_del_buffer, buffer);
				free(buffer);
			} break;

			case FUNCION_SELECT: {
				loguear(debug, logger, "[Conexión] pre deserializar request select");
				t_request_select *buffer = deserializar_request_select(mensaje_de_kernel);
				loguear(info, logger, "Hacer select con [TABLA = %s, KEY = %d]", buffer->tabla, buffer->key);

				char *value_desde_memoria = memoria_select(buffer->tabla, buffer->key);
				if(value_desde_memoria) {
					prot_enviar_mensaje(socket_kernel, FUNCION_SELECT, strlen(value_desde_memoria), value_desde_memoria);
					free(value_desde_memoria);
				}
				else {
					size_t tamanio_del_buffer = sizeof(int) + strlen(buffer->tabla) + sizeof(uint16_t);
					void *buffer_serializado = serializar_request_select(buffer->tabla, buffer->key);
					prot_enviar_mensaje(socket_lissandra, FUNCION_SELECT, tamanio_del_buffer, buffer_serializado);
					free(buffer_serializado);
					t_prot_mensaje *mensaje_de_lissandra = prot_recibir_mensaje(socket_lissandra);
					if(mensaje_de_lissandra->head == REGISTRO_TABLA) {
						loguear(info, logger, "LLego el dato de fs, tabla %s", buffer->tabla);
						size_t tamanio_de_linea = mensaje_de_lissandra->tamanio_total-sizeof(t_header);
						char *linea = malloc(sizeof(char)*(tamanio_de_linea+1));
						memset(linea, 0, tamanio_de_linea);
						memcpy(linea, mensaje_de_lissandra->payload, tamanio_de_linea);
						linea[tamanio_de_linea] = '\0';
						loguear(debug, logger, "Linea: -%s-", linea);
						if(!string_equals_ignore_case(linea, ERROR_NO_EXISTE_TABLA) && !string_equals_ignore_case(linea, ERROR_KEY_NO_ENCONTRADA)) {
							char **separador = string_n_split(linea, 3, ";");

							t_est_tds *segmento = obtener_segmento_por_tabla(buffer->tabla);
							if(!segmento) {
								t_est_tdp *registro = obtener_frame();
								if(registro == NULL) {
									loguear(error, logger, "[Error] No hay suficientes frames =========== NO DEBE PASAR");
									//TODO EJECUTAR ALGORITMO DE REEMPLAZO
								} else {
									crear_asignar_segmento(false, segmento, registro, buffer->tabla, string_to_timestamp(separador[0]), string_to_int16(separador[1]), separador[2]);
								}
							}
							else {
								if(obtener_pagina_por_key(segmento->paginas, string_to_int16(separador[1])) != NULL) {
									//todo no deberia ser posible que teniendo la key, el select vaya a consultar a fs
									loguear(error, logger, "[SELECT TENIA KEY] ESTO DEBERIA PASAR?");
								} else {
									t_est_tdp *registro = obtener_frame();
									t_est_tds *segmento = obtener_segmento_por_tabla(buffer->tabla);
									if(!segmento) {
											crear_asignar_segmento(false, segmento, registro, buffer->tabla, string_to_timestamp(separador[0]), string_to_int16(separador[1]), separador[2]);
									}
									else if(registro == NULL) {
										loguear(error, logger, "[Error] No hay suficientes frames =========== NO DEBE PASAR");
									} else {
										registro->modificado = 0;
										settear_timestamp(registro->ptr_posicion, string_to_timestamp(separador[0]));
										settear_key(registro->ptr_posicion, string_to_int16(separador[1]));
										settear_value(registro->ptr_posicion, separador[2]);
										list_add(segmento->paginas, registro);
									}
								}
							}
							loguear(debug, logger, "[SELECT-RECIBIDO] llego: %s", linea);
							prot_enviar_mensaje(socket_kernel, FUNCION_SELECT, strlen(separador[2]), separador[2]);
							string_iterate_lines(separador, (void*)free);
							free(separador);
							free(linea);
							prot_destruir_mensaje(mensaje_de_lissandra);
						}
						else {
							loguear(debug, logger, "[SELECT-RECIBIDO] llego: %s", linea);
							prot_enviar_mensaje(socket_kernel, FUNCION_SELECT, strlen(linea), linea);
							free(linea);
							prot_destruir_mensaje(mensaje_de_lissandra);
						}
					}
				}
				free(buffer->tabla);
				//free(buffer->key);
				free(buffer);
			} break;

			case FUNCION_INSERT: {
				loguear(debug, logger, "[Conexión] pre deserializar request insert");
				t_request_insert *biffer = deserializar_request_insert(mensaje_de_kernel);
				loguear(info, logger, "Hacer insert con [TABLA = %s, KEY = %d, VALUE = %s, EPOCH = %llu]", biffer->nombre_tabla, biffer->key, biffer->value, biffer->epoch);
				//prot_enviar_mensaje(socket_lissandra, FUNCION_INSERT, mensaje_de_kernel->tamanio_total - sizeof(t_header), mensaje_de_kernel->payload);
				memoria_insert(biffer->nombre_tabla, biffer->key, biffer->value, biffer->epoch);
				free(biffer->nombre_tabla);
				free(biffer->value);
				free(biffer);
			} break;

			case FUNCION_CREATE: {
				loguear(debug, logger, "[Conexión] pre deserializar resquest CREATE");
				t_request_create *buffer = deserializar_request_create(mensaje_de_kernel);
				loguear(info, logger, "Hacer create con [NombreTabla = %s, tipoConsistencia = %s, numeroPart = %d, compatTime = %d]", buffer->nombre_tabla, buffer->tipo_consistencia, buffer->numero_particiones, buffer->compaction_time);
				prot_enviar_mensaje(socket_lissandra, FUNCION_CREATE, mensaje_de_kernel->tamanio_total - sizeof(t_header), mensaje_de_kernel->payload);
				free(buffer->nombre_tabla);
				free(buffer->tipo_consistencia);
				free(buffer);
				t_prot_mensaje *respuesta_create = prot_recibir_mensaje(socket_lissandra);
				if(respuesta_create->head == RESPUESTA_CREATE) {
					prot_enviar_mensaje(socket_kernel, RESPUESTA_CREATE, respuesta_create->tamanio_total - sizeof(t_header), respuesta_create->payload);
				}
				prot_destruir_mensaje(respuesta_create);

			} break;
			case FUNCION_DESCRIBE: {
				loguear(debug, logger, "[Conexión] pre deserializar resquest DESCRIBE");
				size_t tam = mensaje_de_kernel->tamanio_total-sizeof(t_header);
				if(tam == 0){
					loguear(info, logger, "Describe nulo");
				}
				else{
					char* tabla = malloc(tam+1);
					memset(tabla, 0, tam+1);
					memcpy(tabla, mensaje_de_kernel->payload, tam);
					tabla[tam] = '\0';
					loguear(info, logger, "Describe con tabla: %s", tabla);
					free(tabla);
				}
				prot_enviar_mensaje(socket_lissandra, FUNCION_DESCRIBE, mensaje_de_kernel->tamanio_total - sizeof(t_header), mensaje_de_kernel->payload);
				t_prot_mensaje *mensaje_de_lissandra = prot_recibir_mensaje(socket_lissandra);
				prot_enviar_mensaje(socket_kernel, RESPUESTA_DESCRIBE, mensaje_de_lissandra->tamanio_total - sizeof(t_header), mensaje_de_lissandra->payload);
				prot_destruir_mensaje(mensaje_de_lissandra);
			} break;

			case FUNCION_DROP: {
				int tab = mensaje_de_kernel->tamanio_total-sizeof(mensaje_de_kernel->head);
				char* tabla = malloc(tab+1);
				memset(tabla, 0, tab+1 );
				memcpy(tabla, mensaje_de_kernel->payload,tab);
				tabla[tab] = '\0';
				memoria_drop(tabla);
				prot_enviar_mensaje(socket_lissandra, FUNCION_DROP, tab, mensaje_de_kernel->payload);
				loguear(info, logger, "Drop con tabla: %s", tabla);
				free(tabla);

			} break;

			case FUNCION_JOURNAL: {
				journal();
			} break;

			default: {
				cortar_while = true;
				loguear(warning, logger, "Me llegó un mensaje desconocido");
				break;
			}
		}
		prot_destruir_mensaje(mensaje_de_kernel);
		usleep(memoria_config.retardo_accesso_a_fs * 1000); //TODO Este semaforo no va aca, ejemplo: select responde desde mp
	}
}


void iniciar_memoria() {
	memoria = malloc(memoria_config.tamanio_de_memoria);
	loguear(debug, logger, "Inicio de memoria: [Hex]=%p", memoria);
	size_t tamanio_key = sizeof(uint16_t);
	size_t tamanio_timestamp = sizeof(uint64_t);
	loguear(debug, logger, "SizeOf Key: %d, SizeOf Timestamp:%d", tamanio_key, tamanio_timestamp);
	tamanio_de_pagina = tamanio_value + tamanio_key + tamanio_timestamp;
	/*
	 * Descartado porque es probable que por el padding, el total de frames no sea múltipo de 2ALaN, por ende el test_bit te da al menos un frame que no existe.
	int bytes_desde_frames = redondear_hacia_arriba(total_de_frames, 8);
	loguear(debug, logger, "Cantidad Frames / 8 (redondea arriba) = %d", bytes_desde_frames);
	bitmap = calloc(bytes_desde_frames, sizeof(char));
	estado_frames = bitarray_create_with_mode(bitmap, bytes_desde_frames, MSB_FIRST);*/
	frames = list_create();
	tds = list_create();
	int total_de_frames = memoria_config.tamanio_de_memoria / tamanio_de_pagina;
	loguear(debug, logger, "Tamanio de memoria: %d, Tamanio de página: %d, Total de frames: %d", memoria_config.tamanio_de_memoria, tamanio_de_pagina, total_de_frames);
	int i = 1;
	for(;i <= total_de_frames; i++) {
		t_est_tdp *pagina = malloc(sizeof(t_est_tdp));
		pagina->modificado = -1;
		pagina->nro_pag = i;
		pagina->ptr_posicion = memoria + ((i-1) * tamanio_de_pagina);
		pagina->ultima_referencia = 0;
		loguear(debug, logger, "Posicion de memoria pag %d: [Hex]=%p", i, pagina->ptr_posicion);
		list_add(frames, pagina);
	}
}

t_est_tdp *obtener_pagina_por_key(t_list *lista, uint16_t key) {
	bool _buscar_por_frame(t_est_tdp *unaPagina) {
		return obtener_key_de_pagina(unaPagina->ptr_posicion) == key;
	}
	return (t_est_tdp*)list_find(lista, (void*)_buscar_por_frame);
}

t_est_tds *obtener_segmento_por_tabla(char *tabla) {
	bool _buscar_por_nombre(t_est_tds *unSegmento) {
		return string_equals_ignore_case(unSegmento->nombre_segmento, tabla);
	}
	return (t_est_tds*)list_find(tds, (void*)_buscar_por_nombre);
}

t_est_tdp *obtener_frame_libre() {
	bool _buscar_por_pagina_libre(t_est_tdp *unaPagina) {
		return unaPagina->modificado == -1;
	}
	return (t_est_tdp*)list_find(frames, (void*)_buscar_por_pagina_libre);
}

uint16_t obtener_key_de_pagina(void *frame) {
	uint16_t retorno;
	memset(&retorno, 0, sizeof(uint16_t));
	memcpy(&retorno, frame+sizeof(uint64_t), sizeof(uint16_t));
	return retorno;
}

uint64_t obtener_timestamp_de_pagina(void *frame) {
	uint64_t retorno;
	memset(&retorno, 0, sizeof(uint64_t));
	memcpy(&retorno, frame, sizeof(uint64_t));
	return retorno;
}

char *obtener_value_de_pagina(void *frame) {
	char *retorno = malloc(sizeof(char)*tamanio_value+1);
	memset(retorno, 0, tamanio_value+1);
	memcpy(retorno, frame+sizeof(uint64_t)+sizeof(uint16_t), tamanio_value);
	return retorno;
}

void settear_timestamp(void* frame, uint64_t time) {
	memcpy(frame, &time, sizeof(uint64_t));
}

void settear_key(void* frame, uint16_t key) {
	memcpy(frame+sizeof(uint64_t), &key, sizeof(uint16_t));
}

void settear_value(void *frame, char* value) {
	memset(frame+sizeof(uint64_t)+sizeof(uint16_t), 0, tamanio_value);
	memcpy(frame+sizeof(uint64_t)+sizeof(uint16_t), value, strlen(value));
}

void crear_asignar_segmento(bool es_insert, t_est_tds *segmento, t_est_tdp* frame_libre, char *tabla, uint64_t timestamp, uint16_t key, char *value) {
	frame_libre->modificado = es_insert;
	frame_libre->ultima_referencia = get_timestamp();
	loguear(warning, logger, "Frame recibido: %d", frame_libre->nro_pag);
	memset(frame_libre->ptr_posicion, 0, tamanio_de_pagina);
	settear_timestamp(frame_libre->ptr_posicion, timestamp);
	settear_key(frame_libre->ptr_posicion, key);
	settear_value(frame_libre->ptr_posicion, value);
	//strcpy(frame_libre->ptr_posicion+sizeof(int)+sizeof(uint16_t), value);
	if(!segmento) {
		t_est_tds *nuevo_segmento = malloc(sizeof(t_est_tds));
		nuevo_segmento->nombre_segmento = string_duplicate(tabla);
		nuevo_segmento->paginas = list_create();
		list_add(nuevo_segmento->paginas, frame_libre);
		list_add(tds, nuevo_segmento);
	} else {
		list_add(segmento->paginas, frame_libre);
	}
}

void limpiar_segmento(t_est_tds *segmento) {
	/*void _limpiar_paginas(t_est_tdp pagina) {
		free(pagina);
	}*/
	free(segmento->nombre_segmento);
	list_destroy(segmento->paginas);
	free(segmento);
}

void limpiar_tds() {
	list_destroy_and_destroy_elements(tds, (void*)limpiar_segmento);
}

void limpiar_frames() {
	void _limpiar_paginas(t_est_tdp *pagina) {
		free(pagina);
	}
	list_destroy_and_destroy_elements(frames, (void*)_limpiar_paginas);
}

void limpiar_memoria() {
	limpiar_tds();
	limpiar_frames();
	free(memoria);
}

t_est_tdp *obtener_frame() {
	t_est_tdp *un_frame = NULL;
	un_frame = obtener_frame_libre();
	if(un_frame == NULL) {
		un_frame = frame_desde_lru();
		if(un_frame == NULL) {
				loguear(warning, logger, "Memoria llena, efectuando journaling");
				journal();
				pthread_mutex_lock(&mutex_journaling);
				un_frame = obtener_frame();
				pthread_mutex_unlock(&mutex_journaling);
		}
	}
	return un_frame;
}

void printear_memoria() {
	loguear(debug, logger, "-------------------- ESTADO DE LA MEMORIA -------------------------");
	void _imprimir_segmentos(t_est_tds *segmento) {
		loguear(debug, logger, "Segmento: %s", segmento->nombre_segmento);
		void _imprimir_paginas(t_est_tdp *pagina) {
			char *value_de_pagina = obtener_value_de_pagina(pagina->ptr_posicion);
			loguear(debug, logger, "Ut. Ref: %llu | Modificado: %d | Time: %llu | Key: %d | Value: %s",
					pagina->ultima_referencia, pagina->modificado, obtener_timestamp_de_pagina(pagina->ptr_posicion),
					obtener_key_de_pagina(pagina->ptr_posicion), value_de_pagina);
			free(value_de_pagina);
		}
		loguear(debug, logger, "-------------------------------------------------------------------");
		list_iterate(segmento->paginas, (void *)_imprimir_paginas);
	}
	list_iterate(tds, (void *)_imprimir_segmentos);
	loguear(debug, logger, "-------------------------------------------------------------------");
}

t_est_tdp *frame_desde_lru() {
	t_est_tdp *retorno = NULL;
	void _frame_mas_antiguo(t_est_tdp *frame) {
		if(retorno == NULL) {
			if(frame->modificado != 1)
				retorno = frame;
		}
		else if(frame->modificado != 1 && frame->ultima_referencia < retorno->ultima_referencia) {
			retorno = frame;
		}
	}
	list_iterate(frames, (void *)_frame_mas_antiguo);
	return retorno;
}

bool ya_se_conecto_a(char *ip, int puerto) {
	bool _conectado(t_memoria_conectada *memoria) {
		return string_equals_ignore_case(memoria->ip, ip) && memoria->puerto == puerto;
	}
	return list_find(tabla_gossip, (void *)_conectado) != NULL;
}

void cargar_tabla_gossip() {
	if(contar_items(memoria_config.ip_seeds) != contar_items(memoria_config.puerto_seeds)) {
		loguear(error, logger, "[ERROR] No coincide la cantidad de ip y puerto seeds.");
		exit(1);
	}
	loguear(info, logger, "[Gossiping] proceso iniciado");
	int posicion = 0;
	int cero = 0;
	while(memoria_config.ip_seeds[posicion] != NULL) {
		t_memoria_conectada *memoria = malloc(sizeof(t_memoria_conectada));
		//TODO VER ESTO
		memoria->ip = malloc(strlen(memoria_config.ip_seeds[posicion])+1);
		memset(memoria->ip, 0, strlen(memoria_config.ip_seeds[posicion])+1);
		memcpy(memoria->ip, memoria_config.ip_seeds[posicion], strlen(memoria_config.ip_seeds[posicion]));
		memoria->puerto = atoi(memoria_config.puerto_seeds[posicion]);
		memoria->nombre = 0;
		list_add(tabla_gossip, memoria);
		posicion++;
	}
	mostrar_tabla_gossip(tabla_gossip, logger);
}

void iniciar_gossip() {

	t_memoria_conectada *self = malloc(sizeof(t_memoria_conectada));
	//TODO VER ESTO
	self->ip = malloc(strlen(memoria_config.ip_servidor)+1);
	memset(self->ip, 0, strlen(memoria_config.ip_servidor)+1);
	memcpy(self->ip, memoria_config.ip_servidor, strlen(memoria_config.ip_servidor));
	memcpy(&self->puerto, &memoria_config.puerto_escucha, sizeof(int));
	memcpy(&self->nombre, &memoria_config.numero_memoria, sizeof(int));
	list_add(tabla_gossip, self);
	while(!consola_ejecuto_exit) {
		if(contar_items(memoria_config.ip_seeds) != contar_items(memoria_config.puerto_seeds)) {
			loguear(error, logger, "[ERROR] No coincide la cantidad de ip y puerto seeds.");
			exit(1);
		}
		loguear(info, logger, "[Gossiping] proceso iniciado");
		int posicion = 0;
		while(memoria_config.ip_seeds[posicion] != NULL) {
			if(ya_se_conecto_a(memoria_config.ip_seeds[posicion], atoi(memoria_config.puerto_seeds[posicion]))) {
				posicion++;
				continue;
			}
			int socket_memoria_seed = conectar_a_servidor_sin_exit(memoria_config.ip_seeds[posicion], atoi(memoria_config.puerto_seeds[posicion]), MEMORIA);
			if(socket_memoria_seed == -1) {
				loguear(error, logger, "[Gossiping] Falló al conectar con: %s:%d.",memoria_config.ip_seeds[posicion], atoi(memoria_config.puerto_seeds[posicion]));
			}
			else {
				loguear(info, logger, "[Gossiping] Par detectado, comenzando el proceso de intercambio.");
				size_t tamanio_del_buffer = 0;
				void _calcular_buffer(t_memoria_conectada *memoria) {
					tamanio_del_buffer += sizeof(int)*3 + strlen(memoria->ip);
				}
				list_iterate(tabla_gossip, (void*)_calcular_buffer);
				loguear(debug, logger, "[Gossiping] Tamanio del buffer: %d", tamanio_del_buffer);
				void *buffer = serializar_tabla_gossip(tamanio_del_buffer, tabla_gossip);
				//Envio mi tabla, para que otra memoria la reciba.
				prot_enviar_mensaje(socket_memoria_seed, INTERCAMBIAR_TABLA_GOSSIP, tamanio_del_buffer, buffer);
				free(buffer);
				t_prot_mensaje *tabla_gossip_response = prot_recibir_mensaje(socket_memoria_seed);
				t_list *tabla_gossip_recibida = deserializar_tabla_gossip(tabla_gossip_response, logger);
				intercambir_memorias_conectadas(tabla_gossip, tabla_gossip_recibida);
				prot_destruir_mensaje(tabla_gossip_response);
				//En la posicion 0 está self.
				/*int nuevos_seeds = 0;
				if((nuevos_seeds = list_size(tabla_gossip)) > 1) {
					agregar_nuevos_a_seeds(nuevos_seeds);
				}*/

				mostrar_tabla_gossip(tabla_gossip, logger);
			}
			posicion++;
		}
		loguear(info, logger, "[Gossiping] proceso finalizado, durmiendo...");
		usleep(memoria_config.tiempo_gossiping*1000);
	}
}

void reiniciar_frame(t_est_tdp *frame) {
	frame->modificado = -1;
	frame->ultima_referencia = 0;
}

void journal_automatico() {
	while(!consola_ejecuto_exit) {
		loguear(info, logger, "[Journal Automático] Comenzó");
		journal();
		usleep(memoria_config.tiempo_journaling * 1000);
	}
}
/*void agregar_nuevos_a_seeds(int nuevos_seeds) {
	loguear(debug, logger, "nuevos seeds: %d", nuevos_seeds);
	loguear(debug, logger, "Items antes: %d", contar_items(memoria_config.ip_seeds));
	loguear(debug, logger, "Items antes: %d", contar_items(memoria_config.puerto_seeds));
	int seeds = contar_items(memoria_config.ip_seeds);
	memoria_config.ip_seeds = realloc(memoria_config.ip_seeds, sizeof(char*) * (seeds+nuevos_seeds-1));
	memoria_config.puerto_seeds = realloc(memoria_config.puerto_seeds, sizeof(char*) * (seeds+nuevos_seeds-1));
	int indice = 0;
	void _agregar_otros(t_memoria_conectada *memoria) {
		if(!(memoria->puerto == memoria_config.puerto_escucha &&
				string_equals_ignore_case(memoria->ip, memoria_config.ip_servidor))) {
			memoria_config.puerto_seeds[seeds-1+indice] = string_duplicate(string_itoa(memoria->puerto));
			memoria_config.ip_seeds[seeds-1+indice] = string_duplicate(memoria->ip);
			indice++;
		}
	}
	list_iterate(tabla_gossip, (void *)_agregar_otros);
	loguear(debug, logger, "Items despues: %d", contar_items(memoria_config.ip_seeds));
	loguear(debug, logger, "Items despues: %d", contar_items(memoria_config.puerto_seeds));
}*/

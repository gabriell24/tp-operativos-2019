#include "planificacion.h"

void iniciar_listas_planificacion() {
	lista_nuevos = list_create();
	lista_ready = list_create();
	lista_exec = list_create();
	int pos = 0;
	while(pos < kernel_config.multiprocesamiento) {
		estado_exec *estado_exec = malloc(sizeof(estado_exec));
		estado_exec->ocupada = false;
		estado_exec->posicion = pos;
		list_add(lista_exec, estado_exec);
		pos++;
	}
	sem_init(&lqls_en_ready, 0, 0);
	sem_init(&instancias_exec, 0, kernel_config.multiprocesamiento);
	log_debug(logger, "Iniciando planificacion");
	pthread_mutex_init(&mutex_listas, NULL);
	planificar();
	pthread_mutex_destroy(&mutex_listas);
}

/*void crear_execs() {
	for(int i = 0; i < kernel_config.multiprocesamiento; i++) {
		t_list *auxiliar = list_create();
		list_add(lista_exec, auxiliar);
		--*
		 * podria tener dentro de la lista, una esctructura, que marue ocupado
		 * o podria manejarme con el get y el remove
		 *--
	}
}*/

/* Simula la transicion de new a ready del SO */
void admitir_lql(compartir_info_lql *datos){
	t_lql *un_lql = datos->lql;
	un_lql->prox_linea_ejecutar = 0;
	if(datos->desde_consola) {
		int tamanio_linea = strlen(datos->data);
		un_lql->lineas_a_ejecutar = 1;
		un_lql->lineas = list_create();
		list_add(un_lql->lineas, string_duplicate(datos->data));
	}
	else {
		t_lql *un_lql = datos->lql;
		un_lql->lineas = list_create();
		un_lql->prox_linea_ejecutar = 0;
		int lineas_leidas = 0;
		FILE *lql_script = fopen(datos->data, "r");
		if(lql_script != NULL) {
			char *linea = malloc(sizeof(char)*150);
			while (fgets(linea, 150, lql_script) != NULL) {
				lineas_leidas++;
				list_add(un_lql->lineas, string_duplicate(linea));
			}
			free(linea);
			fclose (lql_script);
		}
		un_lql->lineas_a_ejecutar = lineas_leidas;
	}
	//cargar los datos
	list_add(lista_ready, un_lql);
	sem_post(&lqls_en_ready);
	free(datos->data);
	free(datos);
	pthread_detach(pthread_self());
}

void crear_un_lql(bool desde_consola, char *data) {
	t_lql *nuevo_lql = malloc(sizeof(t_lql));
	compartir_info_lql *compartir_info = malloc(sizeof(compartir_info_lql));
	compartir_info->data = string_duplicate(data);
	compartir_info->desde_consola = desde_consola;
	compartir_info->lql = nuevo_lql;
	list_add(lista_nuevos, nuevo_lql);
	pthread_create(&hilo_admision, NULL, (void *)admitir_lql, compartir_info);
}

void planificar() {

	while(!consola_ejecuto_exit) {
		log_debug(logger, "Esperando lqls que lleguen a ready");
		sem_wait(&lqls_en_ready);
		log_debug(logger, "Esperando tener un exec");
		sem_wait(&instancias_exec);
		pthread_t hilos_exec;
		pthread_create(&hilos_exec, NULL, (void *)round_robin, NULL);
	}
}

void round_robin() {
	int ut_consumidas = 0;
	pthread_mutex_lock(&mutex_listas);
	t_lql *lql_a_ejecutar = list_remove(lista_ready, 0);
	pthread_mutex_unlock(&mutex_listas);
	log_debug(logger, "Tomé un lql desde ready");
	int numero_exec_asignado = 0;
	log_debug(logger, "Busco un exec libre");
	pthread_mutex_lock(&mutex_listas);
	numero_exec_asignado = obtener_exec_libre();
	pthread_mutex_unlock(&mutex_listas);
	log_debug(logger, "exec asignado: %d", numero_exec_asignado);
	bool finalizo = false;

	//prot_enviar_mensaje(memoria_destino, ESTAS_FULL, 0, NULL);
	//prot_recibir_mensaje(memoria_destino);
	//No me interesa que responda algo, sino, bloquear con el receive para que que no siga ejecutando, si esta en journal.
	while(!finalizo && ut_consumidas < kernel_config.quantum && lql_a_ejecutar->prox_linea_ejecutar < list_size(lql_a_ejecutar->lineas)) {

		log_debug(logger, "Proxima linea antes de ejecutar: %d", lql_a_ejecutar->prox_linea_ejecutar);
		char *linea_a_ejecutar = list_get(lql_a_ejecutar->lineas, lql_a_ejecutar->prox_linea_ejecutar);
		log_debug(logger, "Ejecuto: %s", linea_a_ejecutar);

		t_parser parser = leer(linea_a_ejecutar);
		if(parser.valido) {
			switch(parser.token) {
				case create: {
					int memoria_destino = get_memoria(NULL);
					if(memoria_destino == -1) {
						finalizo = true;
						break;
					}
					kernel_create(memoria_destino, parser.parametros.create.tabla, parser.parametros.create.tipo_consistencia,
							parser.parametros.create.particiones, parser.parametros.create.compaction_time);
				} break;
				case describe: {
					int memoria_destino = get_memoria(NULL);
					if(memoria_destino == -1) {
						finalizo = true;
						break;
					}
					kernel_describe(memoria_destino, parser.parametros.describe.tabla);
				} break;
				case insert: {
					int memoria_destino = get_memoria(parser.parametros.insert.tabla);
					if(memoria_destino == -1) {
						finalizo = true;
						break;
					}
					kernel_insert(memoria_destino, parser.parametros.insert.tabla, parser.parametros.insert.key,
							parser.parametros.insert.value, parser.parametros.insert.timestamp);
				} break;
				case t_select: {
					int memoria_destino = get_memoria(NULL);
					if(memoria_destino == -1) {
						finalizo = true;
						break;
					}
					kernel_select(memoria_destino, parser.parametros.select.tabla, parser.parametros.select.key);
				} break;
				case add: {
					kernel_add(parser.parametros.add.memoria, parser.parametros.add.tipo_consistencia);
				} break;
				case drop: {
					int memoria_destino = get_memoria(parser.parametros.describe.tabla);
					if(memoria_destino == -1) {
						finalizo = true;
						break;
					}
					kernel_drop(memoria_destino, parser.parametros.drop.tabla);
				} break;

				default: log_warning(logger, "codealo vos e.e");
			}

			if(!finalizo) {
				log_debug(logger, "Ejecutando quantum %d", ut_consumidas);
				lql_a_ejecutar->prox_linea_ejecutar++;
				log_debug(logger, "Proxima linea despues de ejecutar: %d", lql_a_ejecutar->prox_linea_ejecutar);
				ut_consumidas++;
				log_debug(logger, "Fin de cicloo, retardando: %d milisegundos", kernel_config.retardo_ciclo_ejecucion);
				finalizo = lql_a_ejecutar->prox_linea_ejecutar == list_size(lql_a_ejecutar->lineas);
			}
		} else {
			log_warning(logger, "La línea parseada [%s] no es valida, paso el proceso a EXIT", linea_a_ejecutar);
			finalizo = true;
		}
		destruir_parseo(parser);

		usleep(kernel_config.retardo_ciclo_ejecucion * 1000);
	}
	//Se termino el quantum, lo devuelvo a la CR
	pthread_mutex_lock(&mutex_listas);
	corte_quantum(numero_exec_asignado);
	if(!finalizo) {
		list_add(lista_ready, lql_a_ejecutar);
		sem_post(&lqls_en_ready);
	} else {
		exit_lql(lql_a_ejecutar);
	}
	sem_post(&instancias_exec);
	pthread_mutex_unlock(&mutex_listas);
	pthread_detach(pthread_self());
}

void corte_quantum(int nro_exec) {
	//mutex
	estado_exec* estado_exec = list_get(lista_exec, nro_exec);
	estado_exec->ocupada = false;
}

int obtener_exec_libre() {
	int iteraciones = 1;
	bool _libre(estado_exec *estado) {
		log_debug(logger, "Iteracion nro %d ", iteraciones++);
		return !estado->ocupada;
	}
	estado_exec* exec_nro = list_find(lista_exec, (void *)_libre);
	exec_nro->ocupada = true;
	log_debug(logger, "exec elegido %d", exec_nro->posicion);
	return exec_nro->posicion;
}

void exit_lql(t_lql *lql) {
	list_destroy_and_destroy_elements(lql->lineas, (void *)free);
	free(lql);
	log_info(logger, "[Planificacion] Fin de ejecución de un LQL");
}

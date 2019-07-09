#include "interfaz.h"

char *memoria_select(char *tabla, uint16_t key) {
	t_est_tds *segmento = obtener_segmento_por_tabla(tabla);
	if(segmento) {
		t_est_tdp *pagina = obtener_pagina_por_key(segmento->paginas, key);
		if(pagina) {
			pagina->ultima_referencia = get_timestamp();
			loguear(info, logger, "[SELECT] Exitoso desde memoria");
			return obtener_value_de_pagina(pagina->ptr_posicion);
		}
	}
	loguear(debug, logger, "Solicitar segmento y key a lissandra");
	return NULL;
}

void memoria_insert(char *tabla, uint16_t key, char *value, int timestamp) {
	//Journal puede borrar mi segmento, por ende tamb la tdp.
	t_est_tdp *frame_libre = obtener_frame();
	t_est_tds *segmento = obtener_segmento_por_tabla(tabla);
	if(segmento) {
		t_est_tdp *pagina = obtener_pagina_por_key(segmento->paginas, key);
		if(pagina) {
			loguear(info, logger, "Existe la página");
			int timestamp_guardado = obtener_timestamp_de_pagina(pagina->ptr_posicion);
			if(timestamp_guardado > timestamp) {
				loguear(warning, logger, "El timestamp guardado: %d es mas actualizado que %d, para [%s-%d-%s]",
						timestamp_guardado, timestamp, tabla, key, value);
			}
			pagina->modificado = 1;
			pagina->ultima_referencia = get_timestamp();
			settear_timestamp(pagina->ptr_posicion, timestamp);
			settear_value(pagina->ptr_posicion, value);
			loguear(info, logger, "Página actualizada");
		}
		else {
			if(!frame_libre) {
				loguear(info, logger, "::::::::::::MEMORIA LLENA TENGO SEGMENTO, SI ESTAS LEYENDO ESTO ESTAAA MAALLL::::::");
			} else {
				crear_asignar_segmento(true, segmento, frame_libre, tabla, timestamp, key, value);
			}
		}
	}
	else {
		t_est_tdp *frame_libre = obtener_frame();
		if(!frame_libre) {
			loguear(info, logger, "::::::::::::MEMORIA LLENA Y SIN SEGMENTO, SI ESTAS LEYENDO ESTO ESTAAA MAALLL::::::");
			//Acá es necesario hacer un journal
		} else {
			crear_asignar_segmento(true, segmento, frame_libre, tabla, timestamp, key, value);
		}
	}
}

void memoria_create(char *tabla, char *consistencia, int particiones, int compaction_time) {
	void* buffer = serializar_request_create(tabla, consistencia, particiones, compaction_time);

	size_t tamanio_del_paquete = (sizeof(int)*4 + strlen(tabla) + strlen(consistencia));
	prot_enviar_mensaje(socket_lissandra, FUNCION_CREATE, tamanio_del_paquete, buffer);
	free(buffer);

}

void memoria_describe(char *tabla) {
	if(tabla) {
		//Consulta por una tabla
	} else {
		prot_enviar_mensaje(socket_lissandra, FUNCION_DESCRIBE, 0, NULL);
	}
	t_prot_mensaje *mensaje = prot_recibir_mensaje(socket_lissandra);
	t_list *respuesta = deserializar_response_describe(mensaje, logger);
	imprimir_datos_describe(respuesta);
}

void memoria_drop(char *tabla) {
	t_est_tds *segmento = obtener_segmento_por_tabla(tabla);
	if(segmento) {
		bool _eliminar_tabla(t_est_tds *segmento) {
			return string_equals_ignore_case(segmento->nombre_segmento, tabla);
		}
		list_remove_and_destroy_by_condition(tds, (void *)_eliminar_tabla, (void *)limpiar_segmento);
		list_remove_by_condition(tds, (void *)_eliminar_tabla);
	} else {
		loguear(warning, logger, "No existía segmento de %s para eliminar", tabla);
	}
}

/* Las validaciones están porque es posible que no esté llena, y se llame desde la terminal */
void journal() {
	pthread_mutex_lock(&mutex_journaling);
	int indice_segmento = 0;
	void _liberar_segmentos(t_est_tds *segmento) {
		int posicion_pagina = 0;
		void _liberar_modificados(t_est_tdp *pagina) {
			if(pagina->modificado) {
				char *value = obtener_value_de_pagina(pagina->ptr_posicion);
				void *buffer = serializar_request_insert(segmento->nombre_segmento, obtener_key_de_pagina(pagina->ptr_posicion),
						value, obtener_timestamp_de_pagina(pagina->ptr_posicion));

				size_t tamanio_del_paquete = ((strlen(segmento->nombre_segmento) + strlen(value))*sizeof(char)) + (sizeof(int)*3 + sizeof(uint16_t));
				prot_enviar_mensaje(socket_lissandra, FUNCION_INSERT, tamanio_del_paquete, buffer);
				free(buffer);
				free(value);
				list_remove(segmento->paginas, posicion_pagina);
				reiniciar_frame(pagina);
			} else {
				posicion_pagina++;
			}
		}
		list_iterate(segmento->paginas, (void *)_liberar_modificados);
		//list_remove_by_condition(segmento->paginas, (void *)_liberar_modificados);
		loguear(warning, logger, "Tam lista: %d, list is empty: %d", list_size(segmento->paginas), list_is_empty(segmento->paginas));
		if(list_is_empty(segmento->paginas)) {
			list_destroy(segmento->paginas);
			free(segmento->nombre_segmento);
			free(segmento);
			list_remove(tds, indice_segmento);
		} else {
			indice_segmento++;
		}
	}
	list_iterate(tds, (void *)_liberar_segmentos);
	pthread_mutex_unlock(&mutex_journaling);
	usleep(memoria_config.tiempo_journaling * 1000);
}

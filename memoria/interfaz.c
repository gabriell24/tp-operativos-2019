#include "interfaz.h"

char *memoria_select(char *tabla, uint16_t key) {
	t_est_tds *segmento = obtener_segmento_por_tabla(tabla);
	if(segmento) {
		t_est_tdp *pagina = obtener_pagina_por_key(segmento->paginas, key);
		if(pagina) {
			log_info(logger, "[SELECT] Exitoso desde memoria");
			return obtener_value_de_pagina(pagina->ptr_posicion);
		}
	}
	log_debug(logger, "Solicitar segmento y key a lissandra");
	return NULL;
}

void memoria_insert(char *tabla, uint16_t key, char *value, int timestamp) {
	t_est_tds *segmento = obtener_segmento_por_tabla(tabla);
	if(segmento) {
		t_est_tdp *pagina = obtener_pagina_por_key(segmento->paginas, key);
		if(pagina) {
			log_info(logger, "Existe la página");
			int timestamp_guardado = obtener_timestamp_de_pagina(pagina->ptr_posicion);
			if(timestamp_guardado > timestamp) {
				log_warning(logger, "El timestamp guardado: %d es mas actualizado que %s, para [%s-%d-%s]",
						timestamp_guardado, timestamp, tabla, key, value);
			}
			pagina->modificado = 1;
			pagina->ultima_referencia = get_timestamp();
			settear_timestamp(pagina->ptr_posicion, timestamp);
			settear_value(pagina->ptr_posicion, value);
			log_info(logger, "Página actualizada");
		}
		else {
			t_est_tdp *frame_libre = obtener_frame();
			if(!frame_libre) {
				log_info(logger, "::::::::::::MEMORIA LLENA, SI ESTAS LEYENDO ESTO ESTAAA MAALLL::::::");
			} else {
				crear_asignar_segmento(true, segmento, frame_libre, tabla, timestamp, key, value);
			}
		}
	}
	else {
		t_est_tdp *frame_libre = obtener_frame();
		if(!frame_libre) {
			log_info(logger, "::::::::::::MEMORIA LLENA, SI ESTAS LEYENDO ESTO ESTAAA MAALLL::::::");
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
		log_warning(logger, "No existía segmento de %s para eliminar", tabla);
	}
}

/* Las validaciones están porque es posible que no esté llena, y se llame desde la terminal */
void journal() {
	bool _liberar_segmentos(t_est_tds *segmento) {
		int indice = 0;
		void _liberar_modificados(t_est_tdp *pagina) {
			int posicion = 0;
			if(pagina->modificado) {
				char *value = obtener_value_de_pagina(pagina->ptr_posicion);
				void *buffer = serializar_request_insert(segmento->nombre_segmento, obtener_key_de_pagina(pagina->ptr_posicion),
						value, obtener_timestamp_de_pagina(pagina->ptr_posicion));

				size_t tamanio_del_paquete = ((strlen(segmento->nombre_segmento) + strlen(value))*sizeof(char)) + (sizeof(int)*3 + sizeof(uint16_t));
				prot_enviar_mensaje(socket_lissandra, FUNCION_INSERT, tamanio_del_paquete, buffer);
				free(buffer);
				free(value);
				list_remove(segmento->paginas, posicion );

				reiniciar_frame(pagina);
				//return true;
			}
			//return false;
			posicion ++;
		}
		list_iterate(segmento->paginas, (void *)_liberar_modificados);
		//list_remove_by_condition(segmento->paginas, (void *)_liberar_modificados);
		log_warning(logger, "Tam lista: %d, list is empty: %d", list_size(segmento->paginas), list_is_empty(segmento->paginas));
		if(list_is_empty(segmento->paginas)) {
			list_destroy(segmento->paginas);
			free(segmento->nombre_segmento);
			free(segmento);
			list_remove(tds, indice);
		}
		indice ++;
	}
	list_iterate(tds, (void *)_liberar_segmentos);
}

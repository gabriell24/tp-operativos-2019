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
		}
		else {
			t_est_tdp *frame_libre = obtener_frame_libre();
			if(!frame_libre) {
				//Acá es necesario hacer un journal
			} else {
				crear_asignar_segmento(segmento, frame_libre, tabla, timestamp, key, value);
			}
		}
	}
	else {
		t_est_tdp *frame_libre = obtener_frame_libre();
		if(!frame_libre) {
			log_info(logger, "no hay frame libre");
			//Acá es necesario hacer un journal
		} else {
			crear_asignar_segmento(segmento, frame_libre, tabla, timestamp, key, value);
		}
	}
}

void memoria_create(char *tabla, char *consistencia, int particiones, int compaction_time) {

}

void memoria_describe(char *tabla) {
	if(tabla) {
		//Consulta por una tabla
	} else {
		//consulta por todas
	}

}

void memoria_drop(char *tabla) {

}

void journal() {

}

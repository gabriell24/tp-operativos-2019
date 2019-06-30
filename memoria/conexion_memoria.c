#include "conexion_memoria.h"

void escuchar_memoria(int *socket_origen) {
	int socket_memoria = *socket_origen;
	free(socket_origen);
	bool cortar_while = false;
	while (!consola_ejecuto_exit && !cortar_while) {
		t_prot_mensaje *mensaje_de_memoria = prot_recibir_mensaje(socket_memoria);
		switch(mensaje_de_memoria->head) {
			case DESCONEXION: {
				//TODO QUITAR DE LA TABLA_GOSSIP
				loguear(warning, logger, "[Desconexión de memoria] Mato el hilo, ya no podrá recibir mensajes");
				cortar_while = true;
			} break;

			/* Soy memoria B, recibo la tabla de A, la fusiono con la mía, y le mando la mía*/
			case INTERCAMBIAR_TABLA_GOSSIP: {
				t_list *tabla_gossip_recibida = deserializar_tabla_gossip(mensaje_de_memoria, logger);
				intercambir_memorias_conectadas(tabla_gossip, tabla_gossip_recibida);
				mostrar_tabla_gossip();
				//liberar_tabla_gossip(tabla_gossip_recibida);
				size_t tamanio_del_buffer = 0;
				void _calcular_buffer(t_memoria_conectada *memoria) {
					tamanio_del_buffer += sizeof(int)*3 + strlen(memoria->ip);
				}
				list_iterate(tabla_gossip, (void*)_calcular_buffer);
				void *buffer = serializar_tabla_gossip(tamanio_del_buffer, tabla_gossip);
				//Envio mi tabla, para que otra memoria la reciba.
				prot_enviar_mensaje(socket_memoria, INTERCAMBIAR_TABLA_GOSSIP_FIN, tamanio_del_buffer, buffer);
				free(buffer);
				/*int nuevos_seeds = 0;
				if((nuevos_seeds = list_size(tabla_gossip)) > 1) {
					agregar_nuevos_a_seeds(nuevos_seeds);
				}*/

			} break;

			/*Soy memoria A, recibo la tabla procesada de B, y la fusiono con la mía
			case INTERCAMBIAR_TABLA_GOSSIP_FIN: {
				loguear(info, logger, "Fusión final de tabla gossip iniciado");
				t_list *tabla_gossip_recibida = deserializar_tabla_gossip(mensaje_de_memoria, logger);
				intercambir_memorias_conectadas(tabla_gossip, tabla_gossip_recibida);
				liberar_tabla_gossip(tabla_gossip_recibida);
				loguear(info, logger, "Fusión final de tabla gossip finalizado");
			} break;*/

			default: {
				cortar_while = true;
				loguear(warning, logger, "Me llegó un mensaje desconocido");
				break;
			}
		}
		prot_destruir_mensaje(mensaje_de_memoria);
	}
}

void liberar_tabla_gossip(t_list *tabla) {
	void _liberar(t_memoria_conectada *memoria) {
		free(memoria->ip);
		free(memoria);
	}
	list_destroy_and_destroy_elements(tabla, (void *)_liberar);
}

void mostrar_tabla_gossip() {
	loguear(debug, logger, "+--------------- TABLA GOSSIP -------------+");
	void _imprimir(t_memoria_conectada *memoria) {
		loguear(debug, logger, "| Nombre: %d | Ip: %s | Puerto: %d |", memoria->nombre, memoria->ip, memoria->puerto);
	}
	list_iterate(tabla_gossip, (void *)_imprimir);
	loguear(debug, logger, "+------------------------------------------+");
}

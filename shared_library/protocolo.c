#include "protocolo.h"

void prot_enviar_mensaje(int socket_destino,t_header header , size_t tamanio_payload, void* payload){
	int resultado_send = 0;

	//Se arma el paquete de envio
	t_prot_mensaje* mensaje_armado_con_protocolo = (t_prot_mensaje*) malloc(sizeof(t_prot_mensaje));
	mensaje_armado_con_protocolo->head = header;
	mensaje_armado_con_protocolo->payload = malloc(tamanio_payload);
	mensaje_armado_con_protocolo->tamanio_total = sizeof(t_header) + tamanio_payload;

	//Se arma el payload
	memset(mensaje_armado_con_protocolo->payload, 0, tamanio_payload);
	memcpy(mensaje_armado_con_protocolo->payload, payload, tamanio_payload);

	//Envio del tamaño del mensaje, siendo mensaje : header + payload
	resultado_send = send(socket_destino, &(mensaje_armado_con_protocolo->tamanio_total) , sizeof(size_t), 0);

	if( resultado_send <= 0){
		printf("[Conexiones]:\t%s\n",FALLO_ENVIAR_MENSAJE);
		prot_destruir_mensaje(mensaje_armado_con_protocolo);

	}else{
		//Se crea el choclo de bytes de forma que sea : header + payload
		//1. Reservar lugar para crear el buffer
		void* buffer = malloc(mensaje_armado_con_protocolo->tamanio_total);

		//2. Los primeros bytes son para el HEADER
		memcpy(buffer, &(mensaje_armado_con_protocolo->head), sizeof(t_header));

		//3. El resto para el payload
		memcpy(buffer + sizeof(t_header), mensaje_armado_con_protocolo->payload, mensaje_armado_con_protocolo->tamanio_total - sizeof(t_header));

		resultado_send = send(socket_destino, buffer, mensaje_armado_con_protocolo->tamanio_total, 0);

		if( resultado_send <= 0){
			printf("[Conexiones]:\t%s\n",FALLO_ENVIAR_MENSAJE);
		}

		free(buffer);
		prot_destruir_mensaje(mensaje_armado_con_protocolo);
	}
}

t_prot_mensaje* prot_recibir_mensaje(int socket_origen){

	t_prot_mensaje* retorno = (t_prot_mensaje*) malloc(sizeof(t_prot_mensaje));
	/* Si llega 0 es que hubo un close si llega -1 es error*/

	int resultado_recv = recv(socket_origen, &(retorno->tamanio_total), sizeof(size_t), MSG_WAITALL);

	if ( resultado_recv == 0 ){
		close(socket_origen);
		//Si el programa corta aca hay que hacer los free aca porque no continua el flujo
		free(retorno);
		return mensaje_desconexion_al_recibir();
	}

	if ( resultado_recv == -1 ){
		close(socket_origen);
		//Si el programa corta aca hay que hacer los free aca porque no continua el flujo
		free(retorno);
		return mensaje_error_al_recibir();
	}

	void* buffer = malloc(retorno->tamanio_total);

	resultado_recv = recv(socket_origen, buffer, retorno->tamanio_total, MSG_WAITALL);

	if( resultado_recv == 0 ){
		close(socket_origen);
		free(retorno);
		free(buffer);
		return mensaje_desconexion_al_recibir();
	}

	if( resultado_recv == -1 ){
		close(socket_origen);
		free(retorno);
		free(buffer);
		return mensaje_error_al_recibir();
	}

	void* payload = malloc(retorno->tamanio_total - sizeof(t_header));
	t_header header_recibido;

	memcpy( &header_recibido, buffer, sizeof(t_header));
	memcpy( payload, buffer + sizeof(t_header), retorno->tamanio_total - sizeof(t_header));

	retorno->head = header_recibido;
	retorno->payload = payload;

	free(buffer);
	return retorno;
}

void prot_destruir_mensaje(t_prot_mensaje* victima){
	if(victima != NULL){
		if(victima->payload != NULL){
			free(victima->payload);
		}
		free(victima);
	}
}

t_prot_mensaje* mensaje_error_al_recibir(){
	t_prot_mensaje* retorno = (t_prot_mensaje*)  malloc(sizeof(t_prot_mensaje));
	//no hace falta poner el tamanio total en estos casos
	retorno->head = FALLO_AL_RECIBIR;
	retorno->payload = NULL;
	perror("Error al recibir: ");
	return retorno;
}

t_prot_mensaje* mensaje_desconexion_al_recibir(){
	t_prot_mensaje* retorno = (t_prot_mensaje*)  malloc(sizeof(t_prot_mensaje));
	//no hace falta poner el tamanio total en estos casos
	retorno->head = DESCONEXION;
	retorno->payload = NULL;
	return retorno;
}

char* leer_string_de_mensaje(t_prot_mensaje* mensaje){
	/*recordar que siempre que nos envíen un string (independientemente de si es string o stream) dentro de su payload nos mandaran el strlen de su cadena,
	es decir, su cantidad de caracteres. Por lo tanto, se deberá setear (o no por lo dicho anteriormente) el '\0'*/
	size_t largo_string = mensaje->tamanio_total - sizeof(t_header);
	char* string = (char*) malloc(largo_string + 1); // +1 por el '\0'
	memset(string, 0, largo_string);
	memcpy(string, mensaje->payload, largo_string );
	string[largo_string] = '\0';
	return string;
}

#include "conexiones.h"

int levantar_servidor(int puerto){
	int socket_escucha;
	crear_socket(&socket_escucha);

	//Codigo que permite reutilizar inmediatamente el puerto
	int activado = 1;
	setsockopt(socket_escucha, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	//RESERVO PUERTO

	struct sockaddr_in direccion_servidor;
	direccion_servidor.sin_family = AF_INET;
	//necesita un hueco en el sistema operativo, por eso no pasamos ninguna ip en particular
	direccion_servidor.sin_addr.s_addr = INADDR_ANY;
	direccion_servidor.sin_port = htons(puerto);
	//Fin de la burocracia

		//Bind: le pide el puerto declaro al SO, lo reserva para él si esta disponible
	if( bind(socket_escucha, (void*) &direccion_servidor, sizeof(direccion_servidor)) <0){
		close(socket_escucha);
		print_error(ERROR_RESEVAR_PUERTO);
	}


	//ESCUCHO PUERTO

	//Habilita a que los clientes es conecten
	if(listen(socket_escucha, MAX_CLIENTES_ENCOLADOS) <0){
		close(socket_escucha);
		print_error(ERROR_ESCUCHAR_PUERTO);
		exit(VALOR_EXIT);
	}
	return socket_escucha;
}


void print_error(char *mensaje){
	printf("ERROR:\t%s\n", mensaje);
}


void crear_socket(int *socket_destino){
	*socket_destino = socket(AF_INET, SOCK_STREAM, 0);
	//Si no se puede crear socket retorna -
	if(socket_destino <0){
		close(*socket_destino);
		print_error(ERROR_CREAR_SOCKET);
		exit(VALOR_EXIT); //si no se puede crear se cierra el proceso
	}
}


/*
 * Se devuelve el scoket o -1 si tiene se invoca sin el exit,
 * para informar a memoria en el caso de utilizar gossiping
 */
int conectar_servidor(char* ip, int puerto, t_cliente cliente, bool exit_process, int reintentos){
	printf("Conectando a servidor, intento número: %d\n", reintentos);
	int socket_cliente;
	crear_socket(&socket_cliente);

	//conectando el socket

	//Burocracia inicial
	struct sockaddr_in direccion_servidor;
	direccion_servidor.sin_family = AF_INET;
	direccion_servidor.sin_addr.s_addr = inet_addr(ip);
	direccion_servidor.sin_port = htons(puerto);
	//Fin de la burocracia

	//Conectarse con el IP - Puerto del servidor
	if(connect(socket_cliente, (void*) &direccion_servidor, sizeof(direccion_servidor)) < 0){
		if(reintentos < MAX_REINTENTOS_CONEXION) {
			sleep(SEGUNDOS_ESPERA_RECONEXION);
			return conectar_servidor(ip, puerto, cliente, exit_process, reintentos+1);
		}
		close(socket_cliente);
		if(exit_process) {
			print_error(ERROR_CONECTAR_SERVIDOR);
			exit(1);
		}
		else {
			socket_cliente = -1;
		}
	}

	if(socket_cliente != -1) {
		prot_enviar_mensaje(socket_cliente, CONEXION, sizeof(t_cliente), &cliente);
	}

	return socket_cliente;
}


int conectar_a_servidor(char* ip, int puerto, t_cliente cliente){
	return conectar_servidor(ip, puerto, cliente, true, 1);
}

/* Se agrega esta función para cuando falle el gossiping no cierre el proceso,
 * en cambio, devuelve -1 como si hubiese fallado el connect
 */
int conectar_a_servidor_sin_exit(char* ip, int puerto, t_cliente cliente){
	return conectar_servidor(ip, puerto, cliente, false, 1);
}


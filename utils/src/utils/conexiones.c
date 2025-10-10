#include "conexiones.h"

int crearConexion(char *ip, char* puerto)
{
    struct addrinfo hints;
    struct addrinfo *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo(ip, puerto, &hints, &server_info);

    int socket_cliente = socket(server_info->ai_family,
                            	server_info->ai_socktype,
                            	server_info->ai_protocol);

    if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1){
        perror("No se pudo conectar");
        return -1;
    }

    freeaddrinfo(server_info);
    return socket_cliente;
}


int iniciarServidor(char *puerto, char *nombre_servidor,t_log* logger){
	int socket_servidor;

	struct addrinfo hints;
	struct addrinfo *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	// Creo el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family,
							 servinfo->ai_socktype,
							 servinfo->ai_protocol);

	// Asociamos el socket a un puerto
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);
	log_trace(logger, "Listo para escuchar a mi cliente");
	log_info(logger, "Servidor %s listo para recibir clientes...", nombre_servidor);

	return socket_servidor;
}

void *recibir_buffer(uint32_t *size, int socket_cliente){
	void *buffer;

	recv(socket_cliente, size, sizeof(uint32_t), MSG_WAITALL); 
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

// Este libera el buffer y no retorna nada
void recibir_mensaje(int socket_cliente, char *nombreServidor,t_log* logger){
	uint32_t size;
	char *buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "## %s Conectado a %s - FD del socket: %d", buffer,nombreServidor,socket_cliente);
	free(buffer);
}

// ACORDATE DE LIBERAR EL BUFFER BOTON
char* recibir_mensaje_recibido(int socket_cliente, char *nombreServidor){
	uint32_t size;
	return recibir_buffer(&size, socket_cliente);
}


int esperarCliente(int socket_servidor,t_log* logger){ //santi, no se entendio

    // Aceptamos un nuevo cliente
    int socket_cliente = accept(socket_servidor,NULL,NULL);
	if (socket_cliente == -1){
		log_error(logger,"SE PUDRIO TODO, fallo el accept");
		abort();
	}
    log_info(logger, "Se conecto un cliente!");

    return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
    int cod_op;
    if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
        return cod_op;
    else
    {
        close(socket_cliente);
        return -1;
    }
}

void EnviarString(char* mensajito,int socket_servidor,t_log* logger){
	int mensajito_len = string_length(mensajito);
	log_debug(logger,"Debug - (enviarString) - Mensaje Length: %d",mensajito_len);
	send(socket_servidor,&mensajito_len,sizeof(int),0);
	
	log_debug(logger,"Debug - (enviarString) - Mensaje: %s",mensajito);
	send(socket_servidor,mensajito,mensajito_len,0);
}

char* RecibirString(int socket_cliente){
	int mensaje_len;
    recv(socket_cliente, &mensaje_len, sizeof(int), MSG_WAITALL);

    char* mensaje = malloc(mensaje_len);
    recv(socket_cliente, mensaje, mensaje_len, MSG_WAITALL);
	
	return mensaje;
}


#include "conexiones.h"

int crearConexion(char *ip, char* puerto, t_log* logger)
{
    struct addrinfo hints;
    struct addrinfo *server_info;
     struct addrinfo *p;
    int socket_cliente;
    int resultado;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;


    if ((resultado = getaddrinfo(ip, puerto, &hints, &server_info)) != 0) {
        log_error(logger, "Error en getaddrinfo() para puerto %s", puerto);
        return -1;
    }

    for (p = server_info; p != NULL; p = p->ai_next) {
        socket_cliente = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_cliente == -1) {
            continue; 
        }
        if (connect(socket_cliente, p->ai_addr, p->ai_addrlen) != -1) {
            break;
        }
        close(socket_cliente);
    }

    freeaddrinfo(server_info);

    if (p == NULL) {
         log_error(logger, "No se pudo conectar a %s:%s...", ip, puerto);
        return -1;
    }

    log_info(logger, "Conectado exitosamente a %s:%s", ip, puerto);

    
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
    int optval = 1; // para que me de el socket igual

	if (getaddrinfo(NULL, puerto, &hints, &servinfo) != 0) {
        log_error(logger, "Error en getaddrinfo() para puerto %s", puerto);
        return -1;
    }


	socket_servidor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	
	if (socket_servidor == -1) {
        log_error(logger, "Error creando socket: %s", strerror(errno));
        freeaddrinfo(servinfo);
        return -1;
    }


	
    if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        log_error(logger, "Error en setsockopt(SO_REUSEADDR): %s", strerror(errno));
        close(socket_servidor);
        freeaddrinfo(servinfo);
        return -1;
    }


	if (bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        log_error(logger, "Error en bind(): %s", strerror(errno));
        close(socket_servidor);
        freeaddrinfo(servinfo);
        return -1;
    }

	if (listen(socket_servidor, SOMAXCONN) == -1) {
        log_error(logger, "Error en listen(): %s", strerror(errno));
        close(socket_servidor);
        freeaddrinfo(servinfo);
        return -1;
    }

	freeaddrinfo(servinfo);
	log_trace(logger, "Listo para escuchar a mi cliente");
	log_info(logger, "Servidor %s listo para recibir clientes...", nombre_servidor);

	return socket_servidor;
}

void* recibir_buffer(uint32_t *size, int socket_cliente){
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
		log_error(logger,"(esperarCliente) - SE PUDRIO TODO, fallo el accept");
		exit(EXIT_FAILURE);
	}
    log_info(logger, "(esperarCliente) - Se conecto un cliente!");

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

    char* mensaje = malloc(mensaje_len+1);
    recv(socket_cliente, mensaje, mensaje_len, MSG_WAITALL);
	
	return mensaje;
}


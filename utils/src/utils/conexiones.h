#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#define _GNU_SOURCE // anula errores en crearConexion

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>



#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <stdint.h>



int iniciarServidor(char *puerto, char *nombre_servidor,t_log* logger);
int crearConexion(char *ip, char* puerto, t_log* logger);
void recibir_mensaje(int socket_cliente, char *nombreServidor,t_log* logger);
int esperarCliente(int socket_servidor,t_log* logger);
char* recibir_mensaje_recibido(int socket_cliente, char *nombreServidor);
void *recibir_buffer(uint32_t *size, int socket_cliente);
int recibir_operacion(int socket_cliente);
void EnviarString(char* mensajito,int socket_servidor,t_log* logger);
char* RecibirString(int socket_cliente);
//void eliminar_conexion(int ip_cliente, int puerto_cliente);

#endif /* CONEXIONES_H_ */
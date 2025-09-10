#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <stdint.h>
#include <stdint.h>

int iniciar_servidor(char *puerto, char *nombre_servidor,t_log* logger);
int crear_conexion(char *ip, char* puerto);
void recibir_mensaje(int socket_cliente, char *nombreServidor,t_log* logger);

char* recibir_mensaje_recibido(int socket_cliente, char *nombreServidor);
void *recibir_buffer(uint32_t *size, int socket_cliente);


#endif /* CONEXIONES_H_ */
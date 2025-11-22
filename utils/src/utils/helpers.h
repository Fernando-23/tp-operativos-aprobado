#ifndef HELPERS_H_
#define HELPERS_H_

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef enum{
    MENSAJE,
	PAQUETE
}op_code;

typedef enum {
    QUERY,
	MASTER,
	WORKER,
    STORAGE
}Modulo;

typedef enum {
	FILE_INEXISTENTE,
	TAG_INEXISTENTE,
	FILE_PREEXISTENTE,
	TAG_PREEXISTENTE,
	ESPACIO_INSUFICIENTE,
	ESCRITURA_NO_PERMITIDA,
	LECTURA_FUERA_DE_LIMITE,
	ESCRITURA_FUERA_DE_LIMITE
	
}ErrorStorage;

typedef struct {
	u_int32_t size;  
	//int offset; //desplazamiento del payload
	void* stream; 
} t_buffer;

typedef struct {
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct{
    int tamanio_msg;
    char* mensaje;
}t_mensaje;

typedef struct{
    int size;
    char *mensaje;
}Mensaje;

extern const int CANT_MODULOS;
extern const int CANT_ERRORES;

extern char* NOMBRE_MODULOS[CANT_MODULOS];
extern char* NOMBRE_ERRORES[CANT_ERRORES];


t_log* iniciarLogger(char* nombre_modulo, int nivel_log);
t_config* iniciarConfig(char* nombre_config); 
void chequearArgs(int cant_args_ingresados,int limite_cant_args);
void eliminar_paquete(t_paquete *paquete);
void *serializar_paquete(t_paquete *paquete, uint32_t bytes);
void *recibir_buffer(uint32_t *size, int socket_cliente);
t_list *recibir_paquete(int socket_cliente);

int obtenerModuloCodOp(char *string_modulo);

Mensaje* crearMensajito(char* mensaje);
void enviarMensajito(Mensaje* mensaje_a_enviar,int fd,t_log* logger);
Mensaje* recibirMensajito(int socket_cliente);
void liberarMensajito(Mensaje* mensajito_a_liberar);
Mensaje* mensajitoOk();
Mensaje* mensajitoError(ErrorStorage error_a_enviar);

#endif 
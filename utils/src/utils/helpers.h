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
#define CANT_ERRORES 9
#define CANT_MODULOS 4

extern char* path_base_query;

typedef enum {
	MENSAJE,
	PAQUETE,
}op_code_code;

typedef enum {
    QUERY,
	MASTER,
	WORKER,
    STORAGE,
}Modulo;

typedef enum {
	OK,
	FILE_INEXISTENTE,
	TAG_INEXISTENTE,
	FILE_PREEXISTENTE,
	TAG_PREEXISTENTE,
	ESPACIO_INSUFICIENTE,
	ESCRITURA_NO_PERMITIDA,
	LECTURA_FUERA_DE_LIMITE,
	ESCRITURA_FUERA_DE_LIMITE,
}ErrorStorageEnum;

/*typedef enum {
	END,
	READ,
}RespuestaMasterEnum;*/

typedef enum{
	LEER,
	DESALOJAR,
	FINALIZAR,
	ERROR,
	FINALIZAR_WORKER,
}RespuestaEnum;


typedef struct {
	u_int32_t size;  
	//int offset; //desplazamiento del payload
	void* stream; 
} t_buffer;

typedef struct {
	op_code_code codigo_operacion;
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




extern char* NOMBRE_MODULOS[CANT_MODULOS];
extern char* NOMBRE_ERRORES[CANT_ERRORES];
extern char* NOMBRE_RESPUESTA_WORKER[5];

t_log* iniciarLogger(char* nombre_modulo, int nivel_log);
t_config* iniciarConfig(char* nombre_config); 


bool chequearCantArgs(int arg, int cant_esperada);
void chequearCantArgsPasadosPorTerminal(int argc, int cant_esperada);


void eliminar_paquete(t_paquete *paquete);
void *serializar_paquete(t_paquete *paquete, uint32_t bytes);
void *recibir_buffer(uint32_t *size, int socket_cliente);
t_list *recibir_paquete(int socket_cliente);

int obtenerModuloCodOp(char* string_modulo);
int obtenerRespuestaWorkerEnum(char* string_cod_op);

Mensaje* crearMensajito(char* mensaje);
void enviarMensajito(Mensaje* mensaje_a_enviar,int fd,t_log* logger);
Mensaje* recibirMensajito(int socket_cliente, t_log* logger);
void liberarMensajito(Mensaje* mensajito_a_liberar);
Mensaje* mensajitoOk();

#endif 
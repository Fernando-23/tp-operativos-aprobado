#ifndef CICLO_INSTRUCCION_H_
#define CICLO_INSTRUCCION_H_
#include "memoria_interna.h"
#include "helpers-worker.h"



typedef enum{
    CREATE,
    TRUNCATE,
    WRITE,
    READ,
    TAG,
    COMMIT,
    FLUSH,
    DELETE,
    END
}op_code;

typedef enum{
    LRU,
    CLOCK_M
}algoritmo_reemplazo;

typedef struct{
    op_code cod_op;
    char *operacion;
}t_instruccion;

char* Fetch();
void Decode(char* instruccionCom);
bool Execute();

void destruir(void* elemento);

t_list *crearListaDeInstrucciones();

void ejecutarCreate(char* file, char* tag);
void ejecutarTruncate(char* file, char* tag, int tamanio);
void ejecutarWrite(char* file, char* tag, int dir_base, char* contenido);
void ejecutarRead(char* file, char* tag, int dir_base, int tamanio);
void ejecutarTag(char* file_origen, char* tag_origen, char* file_destino, char* tag_destino);
void ejecutarCommit(char* file, char* tag);
void ejecutarFlush(char* file, char* tag);
void ejecutarDelete(char* file, char* tag);
void ejecutarEnd(); 

int obtenerOperacionCodOp(char *string_operacion);

extern t_instruccion* instruccion;
extern pthread_mutex_t sem_instruccion;

extern int socket_master;
extern int socket_storage;

extern int CANT_OPERACIONES_WORKER;
extern char* NOMBRE_OPERACIONES[9];


#endif /* CICLO_INSTRUCCION_H_ */
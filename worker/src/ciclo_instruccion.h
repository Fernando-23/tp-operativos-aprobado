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
char** Decode(char* instruccionCom);
bool Execute(char** instruccion_separada);

void destruir(void* elemento);

t_list *crearListaDeInstrucciones();

bool ejecutarCreate(char* file, char* tag);
bool ejecutarTruncate(char* file, char* tag, int tamanio);
bool ejecutarWrite(char* file, char* tag, int dir_base, char* contenido);
bool ejecutarRead(char* file, char* tag, int dir_base, int tamanio);
bool ejecutarTag(char* file_origen, char* tag_origen, char* file_destino, char* tag_destino);
bool ejecutarCommit(char* file, char* tag);
bool ejecutarFlush(char* file, char* tag);
bool ejecutarDelete(char* file, char* tag);
bool ejecutarEnd(); 

int obtenerOperacionCodOp(char *string_operacion);

extern t_instruccion* instruccion;


extern int socket_master;
extern int socket_storage;

extern int CANT_OPERACIONES_WORKER;
extern char* NOMBRE_OPERACIONES[9];


#endif /* CICLO_INSTRUCCION_H_ */
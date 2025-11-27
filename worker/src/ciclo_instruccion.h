#ifndef CICLO_INSTRUCCION_H_
#define CICLO__INSTRUCCION_H_
 
#include "helpers-worker.h"
#include "memoria_interna.h"

typedef struct{
    op_code cod_op;
    char *operacion
}t_instruccion;

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

extern t_instruccion* instruccion;
extern pthread_mutex_t sem_instruccion;

t_list* crear_lista();

extern int socket_master;
extern int socket_storage;

extern int CANT_OPERACIONES_WORKER;
extern char* NOMBRE_OPERACIONES[9];

char* Fetch();
void Decode(char* instruccionCom);
bool Execute();
void destruir(void* elemento);
int escribir_en_memoria_paginada(char* file, char* tag, int pagina, int desplazamiento, char* contenido);
char* leer_en_memoria_paginada(char* file, char* tag, int pagina, int desplazamiento,int tamanio);
void ejecutarCreate(char* file, char* tag);
void ejecutarTruncate(char* file, char* tag, int tamanio);
void ejecutarWrite(char* file, char* tag, int dir_base, char* contenido);
void ejecutarRead(char* file, char* tag, int dir_base, int tamanio);
void ejecutarTag(char* file_origen, char* tag_origen, char* file_destino, char* tag_destino);
void ejecutarCommit(char* file, char* tag);
void ejecutarFlush(char* file, char* tag);
void ejecutarDelete(char* file, char* tag);
void ejecutarEnd(); 
void frameAStorage(char* file, char* tag, int nro_pag);
EntradaDeTabla* buscar_o_crear_entrada_pagina(TablaPaginas* tabla, int pag_actual);
EntradaDeTabla* buscar_entrada_pagina(TablaPaginas* tabla, int pag_actual);
Frame* buscar_frame_libre();
int obtenerOperacionCodOp(char *string_operacion);

#endif /* CICLO_INSTRUCCION_H_ */
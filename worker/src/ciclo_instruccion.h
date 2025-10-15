#ifndef CICLO_INSTRUCCION_H_
#define CICLO__INSTRUCCION_H_
 
#include "helpers-worker.h"

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

char* Fetch(t_list* lista_instrucciones);
void Decode(char* instruccionCom);
bool Execute();
void destruir(void* elemento);
int escribir_en_memoria_paginada(char* file, char* tag, int pagina, int desplazamiento, char* contenido);
char* leer_en_memoria_paginada(char* file, char* tag, int pagina, int desplazamiento,int tamanio);
void ejecutar_create(char* file, char* tag);
void ejecutar_truncate(char* file, char* tag, int tamanio);
void ejecutar_write(char* file, char* tag, int dir_base, char* contenido);
void ejecutar_read(char* file, char* tag, int dir_base, int tamanio);
void ejecutar_tag(char* file_origen, char* tag_origen, char* file_destino, char* tag_destino);
void ejecutar_commit(char* file, char* tag);
void ejecutar_flush(char* file, char* tag);
void ejecutar_delete(char* file, char* tag);
void ejecutar_end(); 
void frameAStorage(char* file, char* tag, int nro_pag);
entrada_pagina_t* buscar_o_crear_entrada_pagina(tabla_paginas_t* tabla, int pag_actual);
entrada_pagina_t* buscar_entrada_pagina(tabla_paginas_t* tabla, int pag_actual);
frame_t* buscar_frame_libre();
int obtenerOperacionCodOp(char *string_operacion);

#endif /* CICLO_INSTRUCCION_H_ */
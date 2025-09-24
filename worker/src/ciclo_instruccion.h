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

extern t_instruccion* instruccion;

t_list* crear_lista();

extern int socket_master;
extern int socket_storage;

char* Fetch(t_list* lista_instrucciones);
void Decode(char* instruccionCom);
bool Execute();
void destruir(void* elemento);
void escribir_en_memoria(char* file, char* tag, int pagina, size_t desplazamiento, char* contenido);
#endif /* CICLO_INSTRUCCION_H_ */
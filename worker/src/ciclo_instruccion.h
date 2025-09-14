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

t_list* crear_lista();
char* Fetch(t_list* lista_instrucciones);
void Decode(char* instruccionCom);
bool Execute();
void destruir(void* elemento);
#endif /* CICLO_INSTRUCCION_H_ */
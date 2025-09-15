#ifndef QUERYS_H_
#define QUERYS_H_
#include "helpers-master.h"
#include <commons/collections/list.h>
#include <stdlib.h>

typedef struct{
    char* query;
    int prioridad;
    int quid;
}QCB;

void InicializarQueryPorEstado(t_list* lista);

#endif // QUERYS_H_
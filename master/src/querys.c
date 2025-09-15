#include "querys.h"

void InicializarQueryPorEstado(t_list* lista){
    for(int i=0; i<cant_estados; i++){
        QueryPorEstado[i] = list_create();
    }
}

void CrearQCB(char* query, int prioridad){
    //LIBERAR MEMORIA DE QCB
    QCB* qcb = malloc(sizeof(QCB));
    qcb->query = query;
    qcb->prioridad = prioridad;
    qcb->quid = quid_global;

    //wait
    quid_global++;
    //signal

    list_add(QueryPorEstado[READY], qcb);
    log_debug(logger_master, "Debug - (CrearQCB) - Se creo el QCB con Quid: %d, Prioridad: %d y Query: %s", qcb->quid, qcb->prioridad, qcb->query);

}
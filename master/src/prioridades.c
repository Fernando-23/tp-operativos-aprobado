#include "prioridades.h"



Worker* buscarWorkerLibre(){

    Worker* libre = NULL;
    for (int i = 0; i < list_size(lista_workers); i++) {
        Worker* w = (Worker *)list_get(lista_workers, i);
        if (w->esta_libre && w->query_pendiente == NULL) {
            libre = w;
            break;
        }
    }
    return libre;
}

Worker* buscarVictimaDesalojable(int prioridad_nueva) {
    
    Worker* victima = NULL;
    int peor_prio = -1; 

    for (int i = 0; i < list_size(lista_workers); i++) {
        Worker* w = list_get(lista_workers, i);
        if (!w->esta_libre && 
            w->query_actual && 
            w->query_pendiente == NULL &&  
            w->query_actual->prioridad > prioridad_nueva && 
            w->query_actual->prioridad > peor_prio) {       
            victima = w;
            peor_prio = w->query_actual->prioridad;
        }
    }
   
    return victima;
}

Worker* buscarWorkerConMenorPrioridad() {
    Worker* victima = NULL;
    int peor_prio = -1;

    for (int i = 0; i < list_size(lista_workers); i++) {
        Worker* w = list_get(lista_workers, i);

        if (!w) continue;

    
        if (w->esta_libre) continue;
        if (w->query_actual == NULL) continue;
        if (w->query_pendiente != NULL) continue;

        int prio = w->query_actual->prioridad;

        if (prio > peor_prio) {
            peor_prio = prio;
            victima = w;
        }
    }

    return victima;
}

bool sigueEnReady(int quid_a_consultar){
    if (buscarQueryPorIdListaReady(quid_a_consultar)!= NULL){
        log_debug(logger_master,"(sigueEnReady) - Query %d sigue en READY",quid_a_consultar);
        return true;
    }
    log_debug(logger_master,"(sigueEnReady) - Query %d NO sigue en READY",quid_a_consultar);
    return false;
}



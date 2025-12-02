#include "prioridades.h"



Worker* buscarWorkerLibre(){


    Worker* libre = NULL;
    for (int i = 0; i < list_size(lista_workers); i++) {
        Worker* w = list_get(lista_workers, i);
        if (w->esta_libre && w->query_pendiente == NULL) {
            libre = w;
            break;
        }
    }
    return libre;
}

Worker* buscarVictimaDesalojable(int prioridad_nueva) {
    pthread_mutex_lock(&mutex_workers);
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
    pthread_mutex_unlock(&mutex_workers);
    return victima;
}

Worker* buscarWorkerConMenorPrioridad(){
    Worker* victima = NULL;
    int prioridad_menor = -1;

    for (int i = 0; i < list_size(lista_workers); i++) {
        Worker* w = list_get(lista_workers, i);
            int prio_actual = w->query_actual->prioridad;
            if (prioridad_menor == -1 || prio_actual > prioridad_menor) {
                prioridad_menor = prio_actual;
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


void* realizarAgingIndividual(void *args){
    Query* query_gestionando = (Query *)args;
    while (1)
    {
        usleep(config_master->tiempo_aging * 1000);
        if (sigueEnReady(query_gestionando->quid)/*&& !query_gestionando->ya_estuvo_en_ready*/){
            query_gestionando->prioridad--;
            log_info(logger_master, "%d Cambio de prioridad: %d - %d", query_gestionando->quid, query_gestionando->prioridad + 1, query_gestionando->prioridad);
            //probablemente tenga q replanificar
        }else{
            log_debug(logger_master,"(realizarAgingIndividual) - Query %d ya no sigue en ready, terminando thread..."
            ,query_gestionando->quid);
            break;
        }
    }
    return NULL;
    
}
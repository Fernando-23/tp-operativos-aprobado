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
        log_info(logger_master, "(realizarAgingIndividual) - Thread aging iniciado cada %d ms", config_master->tiempo_aging);
        usleep(config_master->tiempo_aging * 1000);
        pthread_mutex_lock(&mutex_workers);
        pthread_mutex_lock(&mutex_lista_ready);
        if (sigueEnReady(query_gestionando->quid)/*&& !query_gestionando->ya_estuvo_en_ready*/){
            query_gestionando->prioridad--;
            log_info(logger_master, "(realizarAgingIndividual) - %d Cambio de prioridad: %d - %d", query_gestionando->quid, query_gestionando->prioridad + 1, query_gestionando->prioridad);

            list_sort(lista_ready, ordenarPorPrioridad);
            Query* primer_elemento = list_get(lista_ready,0);
            
            if(primer_elemento->quid == query_gestionando->quid){
                log_debug(logger_master,"(realizarAgingIndividual) - Soy el primero en READY despues de ordenar");
                intentarEnviarQueryAExecutePorPrioridades(query_gestionando);
            }
            
        }else{
            log_debug(logger_master,"(realizarAgingIndividual) - Query %d ya no sigue en ready, terminando thread..."
            ,query_gestionando->quid);
            pthread_mutex_unlock(&mutex_lista_ready);
            pthread_mutex_unlock(&mutex_workers);
            break;
        }

        pthread_mutex_unlock(&mutex_lista_ready);
        pthread_mutex_unlock(&mutex_workers);
    }
    return NULL;
    
}

void* hiloAging(void* args){
    int intervalo_ms = config_master->tiempo_aging;

    struct timespec ts;
    ts.tv_sec = intervalo_ms / 1000;
    ts.tv_nsec = (intervalo_ms % 1000) * 1000000L;

    while(1){
        nanosleep(&ts, NULL);

        pthread_mutex_lock(&mutex_lista_ready);

        if (list_is_empty(lista_ready)) { // se duerme un ratito
            pthread_mutex_unlock(&mutex_lista_ready);
            continue;
        }

        for (int i = 0; i < list_size(lista_ready); i++) { // bajar prioridadE
            Query* q = list_get(lista_ready, i);
            if (q->prioridad > 0) {
                q->prioridad--;
                log_info(logger_master, "%d Cambio de prioridad: %d - %d", q->quid, q->prioridad + 1, q->prioridad);
            }
        }
            list_sort(lista_ready, ordenarPorPrioridad);
            intentarPlanificarDesdeReady();
        
        pthread_mutex_unlock(&mutex_lista_ready);
    }
    return NULL;
}
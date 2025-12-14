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

Worker* buscarWorkerConMenorPrioridad(){
    Worker* viSSStima = NULL;
    int prioridad_menor = -1;

    for (int i = 0; i < list_size(lista_workers); i++) {
        Worker* w = list_get(lista_workers, i);
        // Verificar que el worker tenga una query en ejecución y no tenga query pendiente
        if (w->query_actual != NULL && w->query_pendiente == NULL && !w->esta_libre) {
            int prio_actual = w->query_actual->prioridad;
            log_debug(logger_master,"(buscarWorkerConMenorPrioridad) - Prioridad actual del Worker %d es %d de Query %d",w->id, prio_actual, w->query_actual->quid);
            if (prioridad_menor == -1 || prio_actual > prioridad_menor) {
                prioridad_menor = prio_actual;
                viSSStima = w;
            }
        }  
    }
    return viSSStima;
}

bool sigueEnReady(int quid_a_consultar){
    // Esta función debe ser llamada con mutex_lista_ready tomado
    // No tomamos el mutex aquí para evitar deadlocks
    Query* query = buscarQueryPorIdListaReady(quid_a_consultar);
    if (query != NULL){
        log_debug(logger_master,"(sigueEnReady) - Query %d sigue en READY",quid_a_consultar);
        return true;
    }
    log_debug(logger_master,"(sigueEnReady) - Query %d NO sigue en READY",quid_a_consultar);
    return false;
}



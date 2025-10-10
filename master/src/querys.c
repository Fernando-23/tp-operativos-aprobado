#include "querys.h"

int quid_global = 0;

Query* crearQuery(char* query, int prioridad,int fd){
    //LIBERAR MEMORIA DE QCB
    
    Query* query_nueva_creada_hoy = malloc(sizeof(Query));
    query_nueva_creada_hoy->query = query;
    query_nueva_creada_hoy->prioridad = prioridad;
    query_nueva_creada_hoy->quid = quid_global;
    query_nueva_creada_hoy->fd = fd;

    pthread_mutex_lock(&mutex_quid_global);
    quid_global++;
    pthread_mutex_unlock(&mutex_quid_global);
    
    log_debug(logger_master, "Debug - (crearQuery) - Se creo la QUery con Quid: %d, Prioridad: %d y Query: %s", query_nueva_creada_hoy->quid, query_nueva_creada_hoy->prioridad, query_nueva_creada_hoy->query);
    return query_nueva_creada_hoy;
}


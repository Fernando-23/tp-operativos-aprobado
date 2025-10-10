#include "helpers-master.h"


int nivel_multiprocesamiento = 0;
ConfigMaster* config_master = NULL;
t_log* logger_master = NULL;

void cargarConfigMaster(char* path_config){
    char* path_completo = string_new();
    string_append(&path_completo, "../configs/");
    string_append(&path_completo, path_config);

    t_config* config = config_create(path_completo);
    config_master = malloc(sizeof(ConfigMaster));
    if(config_master == NULL){
        abort();
    }
    
    config_master->puerto_escucha = string_duplicate(config_get_string_value(config, "PUERTO_ESCUCHA"));
    config_master->algoritmo_plani = string_duplicate(config_get_string_value(config, "ALGORITMO_PLANIFICACION"));
    config_master->tiempo_aging = config_get_int_value(config, "TIEMPO_AGING");
    config_master->log_level = config_get_int_value(config, "LOG_LEVEL");
    
    free(path_completo);
    config_destroy(config);
}

void* atenderClientes(void *args){
    int socket = *(int*)args;
    
    while(1){
        int *fd_cliente = malloc(sizeof(int));
        *fd_cliente = esperarCliente(socket,logger_master);
        

        pthread_t thread_query;
        pthread_create(&thread_query, NULL, gestionarClienteIndividual, (void *)fd_cliente);
        pthread_detach(thread_query); //creo que no hace falta, pero pensar a futuro
        
    }
    
}

void* gestionarClienteIndividual(void* args){
    int *fd_conexion = (int *)args; //clarisimo mi rey
    
    Mensaje* mensaje_a_recibir = recibirMensajito(fd_conexion);
    log_debug(logger_master,
        "Debug - (gestionarClienteIndividual) - Recibi el mensaje %s",mensaje_a_recibir->mensaje);

    char** mensajito_cortado = string_split(mensaje_a_recibir->mensaje," ");
    int query_o_worker = obtenerModuloCodOp(mensajito_cortado[0]);
    
        
    switch (query_o_worker){
        
    case QUERY: //0
        //QUERY nombre_query prioridad
        //--->args:nombre_query prioridad fd_cliente
        char* nombre_query = string_duplicate(mensajito_cortado[1]);
        int prioridad = atoi(mensajito_cortado[2]); 
        
        string_array_destroy(mensajito_cortado);

        gestionarQueryIndividual(nombre_query, prioridad, *fd_conexion); 
        free(fd_conexion);
        break;
    case WORKER: //2

        int id_worker = atoi(mensajito_cortado[1]);
    
        string_array_destroy(mensajito_cortado);

        gestionarWorkerIndividual(id_worker,*fd_conexion);
        break;
    default:
        log_error(logger_master,"Error - (gestionarClienteIndividual) - Codigo de operacion erroneo"); //capaz un abort, quien sabe
        string_array_destroy(mensajito_cortado);
        break;
    }
    
    liberarMensajito(mensaje_a_recibir);
}


void gestionarQueryIndividual(char *nombre_query,int prioridad,int fd){
    Query* query_devuelta_por_funcion_que_crea_query_y_la_devuelve = crearQuery(nombre_query,prioridad,fd);
    int id_query = query_devuelta_por_funcion_que_crea_query_y_la_devuelve->quid;

    pthread_mutex_lock(&mutex_workers);
    pthread_mutex_lock(&mutex_lista_ready);
    bool estaba_vacia_antes_de_que_yo_entre_en_ready = list_is_empty(lista_ready);
    list_add(lista_ready, query_devuelta_por_funcion_que_crea_query_y_la_devuelve); 

    //ESTO CUBRE FIFO
    if(estaba_vacia_antes_de_que_yo_entre_en_ready){
        intentarEnviarQueryAExecute(query_devuelta_por_funcion_que_crea_query_y_la_devuelve);
        pthread_mutex_unlock(&mutex_lista_ready);
        pthread_mutex_unlock(&mutex_workers);
        return;
    }


    //PRIORIDADES
    if (config_master->algoritmo_plani == "PRIORIDADES"){
        
        list_sort(lista_ready,ordenarPorPrioridad);
        Query* primer_elemento = list_get(lista_ready,0);
        
        if (id_query == primer_elemento->quid){ 
            
            /*
            funcion ir viendo worker por worker si alguno tiene mayor prioridad que el 
            primer elemento de la lista ready 

            desaloja
            query 
            porque volviste
            tenes que desalojar 

            si yo mando una interrupcion 
            como hago siendo yo la proxima query a elegir que cuando se libere un worker no me elija
            como hago siendo yo el worker a desalojar que si viene un query con mayor proridad no me vuelva a desalojar
            
            */
        } 
        pthread_mutex_unlock(&mutex_lista_ready);
        pthread_mutex_unlock(&mutex_workers);
        return;
    }
    
    pthread_mutex_unlock(&mutex_lista_ready);
    pthread_mutex_unlock(&mutex_workers);
    
}

//despacha si hay worker libre
void intentarEnviarQueryAExecute(Query *query_que_quiere_laburar){
    
    Worker* laburito_disponible = (Worker *)list_find(workers,buscarLaburanteSinLaburo); //probar esto che
    if (laburito_disponible!=NULL){
       laburito_disponible->query = query_que_quiere_laburar; 
       laburito_disponible->esta_libre=false;
       log_debug(logger_master,"Debug - (intentarEnviarQueryAExecute) - Query %d consiguio laburo con el worker %d",
       query_que_quiere_laburar->quid,laburito_disponible->id);
    }

    
}

bool ordenarPorPrioridad(void *query_vigente_void,void* query_desafiante_void){
    Query* query_vigente = (Query*)query_vigente_void;
    Query* query_desafiante = (Query*)query_desafiante_void;
    return query_vigente->prioridad < query_desafiante->prioridad;
}

void gestionarWorkerIndividual(int id_worker ,int fd_conexion){
    Worker* nuevo_laburante_devuelto_por_la_funcion_que_crea_y_devuelve_worker = crearWorker(id_worker,fd_conexion);    //
    if (!nuevo_laburante_devuelto_por_la_funcion_que_crea_y_devuelve_worker){
        log_error(logger_master,"Error - (gestionarWorkerIndividual) - Error al reservar memoria al reservar memoria");
        return;
    } 
    
    intentarEnviarQueryAExecutePorWorker(nuevo_laburante_devuelto_por_la_funcion_que_crea_y_devuelve_worker);
    
    pthread_mutex_lock(&mutex_workers);
    list_add(workers, nuevo_laburante_devuelto_por_la_funcion_que_crea_y_devuelve_worker);
    pthread_mutex_unlock(&mutex_workers);

}

// no se reserva nunca worker

/*
prioridades: si no hay workers libres y justo nuestro worker es el de menor prioridad


*/

void intentarEnviarQueryAExecutePorWorker(Worker* worker){ // linkedin
    
    pthread_mutex_lock(&mutex_lista_ready);

    if (hayLaburo(lista_ready)){
        Query* query = (Query *)list_remove(lista_ready,0);
        agarrarLaPala(worker,query);
        
        log_debug(logger_master,
        "Debug - (intentarEnviarQueryAExecutePorWorker) - Lista READY no esta vacia. Query %d enviada a Worker %d",query->quid,worker->id);

    } else {
        worker->esta_libre = true;     
        
    }
    pthread_mutex_unlock(&mutex_lista_ready);
}


Worker* crearWorker(int id_worker_a_crear_ahora, int contacto_empleado){
    Worker* nuevo_laburante_que_reserva_memoria_en_el_espacio_heap = malloc(sizeof(Worker));
    nuevo_laburante_que_reserva_memoria_en_el_espacio_heap->esta_libre = true;
    nuevo_laburante_que_reserva_memoria_en_el_espacio_heap->id = id_worker_a_crear_ahora;
    nuevo_laburante_que_reserva_memoria_en_el_espacio_heap->query = NULL;
    nuevo_laburante_que_reserva_memoria_en_el_espacio_heap->fd = contacto_empleado;
    return nuevo_laburante_que_reserva_memoria_en_el_espacio_heap; 
} 

bool hayLaburo(t_list* lista){
    return !(list_is_empty(lista));
}

void agarrarLaPala(Worker* laburador,Query* laburo){
        laburador->query = laburo;
        laburador->esta_libre = false;
}

bool buscarLaburanteSinLaburo(void *args){
    Worker* laburante = (Worker *)args;
    return laburante->esta_libre;
}
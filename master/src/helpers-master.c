#include "helpers-master.h"


int nivel_multiprocesamiento = 0;
ConfigMaster* config_master = NULL;
t_log* logger_master = NULL;

void CargarConfigMaster(char* path_config){
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
        int fd_cliente = esperarCliente(socket);

        pthread_t thread_query;
        pthread_create(&thread_query,NULL,gestionarClienteIndividual,(void *)&fd_cliente);
        pthread_detach(thread_query); //creo que no hace falta, pero pensar a futuro
        
    }
    
}

void* gestionarClienteIndividual(void* args){
    int fd_cliente = *(int*)args; //clarisimo mi rey
    
    Mensaje* mensaje_a_recibir = recibirMensajito(fd_cliente);
    printf("Voy a hacer el biribara \n");
    char** mensajito_cortado = string_split(mensaje_a_recibir->mensaje," ");
    int query_o_worker = atoi(mensajito_cortado[0]);
    
        
    switch (query_o_worker){
        
    case QUERY: //0
        //0 nombre_query prioridad
        //--->args:nombre_query prioridad fd_cliente
        char* nombre_query = string_duplicate(mensajito_cortado[1]);
        int prioridad = atoi(mensajito_cortado[2]); 
        
        string_array_destroy(mensajito_cortado);

        gestionarQueryIndividual(nombre_query, prioridad, fd_cliente); 
        break;
    case WORKER: //2

        int id_worker = atoi(mensajito_cortado[1]);
    
        string_array_destroy(mensajito_cortado);

        gestionarWorkerIndividual(id_worker);
        break;
    default:
        //loggggggg
        string_array_destroy(mensajito_cortado);
        break;
    }
    char* path_query = mensajito_cortado[0];
    int prioridad = atoi(mensajito_cortado[1]);
    
    printf("LLego una query, path query:%s prioridad:%d\n",path_query,prioridad);
    
    liberarMensajito(mensaje_a_recibir);
}


void gestionarQueryIndividual(char *nombre_query,int prioridad,int fd){
    Query* query_devuelta_por_funcion_que_crea_query_y_la_devuelve = crearQuery(nombre_query,prioridad,fd);
    
    
    if (config_master->algoritmo_plani == "PRIORIDADES"){
        // list_sort(lista_ready,); a implementar
        pthread_mutex_lock(&mutex_lista_ready);
        list_add(lista_ready, query_devuelta_por_funcion_que_crea_query_y_la_devuelve);
        pthread_mutex_unlock(&mutex_lista_ready); 
        
        intentarEnviarQueryAExecute(query_devuelta_por_funcion_que_crea_query_y_la_devuelve);
        log_debug(logger_master,"MOCK - (gestionarQueryIndividual) - ordene");
        return;
    }
    //FIFO
    pthread_mutex_lock(&mutex_lista_ready);
    bool estaba_vacia_antes_de_que_yo_entre_en_ready = list_is_empty(lista_ready);
    list_add(lista_ready, query_devuelta_por_funcion_que_crea_query_y_la_devuelve); 
    pthread_mutex_unlock(&mutex_lista_ready);
    
    if(estaba_vacia_antes_de_que_yo_entre_en_ready){
        intentarEnviarQueryAExecute(query_devuelta_por_funcion_que_crea_query_y_la_devuelve);

    }


    
}

//despacha si hay worker libre
void intentarEnviarQueryAExecute(Query *query_que_quiere_laburar){
    pthread_mutex_lock(&mutex_workers);
    Worker* laburito_disponible = (Worker *)list_find(workers,NULL); //probar esto che
    if (laburito_disponible!=NULL){
       //mutex_lista_ready
       laburito_disponible->query = query_que_quiere_laburar; 
       laburito_disponible->esta_libre=false;
       log_debug(logger_master,"Debug - (intentarEnviarQueryAExecute) - Query %d consiguio laburo con el worker %d",
       query_que_quiere_laburar->quid,laburito_disponible->id);
    }

    pthread_mutex_unlock(&mutex_workers);
}
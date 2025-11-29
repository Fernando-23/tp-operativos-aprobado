#include "helpers-master.h"
#include "querys.h"
#include "prioridades.h"

const int nivel_multiprocesamiento = 0;

ConfigMaster* config_master = NULL;
t_log* logger_master = NULL;

t_list* lista_workers = NULL;
t_list* lista_ready = NULL;
pthread_mutex_t mutex_lista_ready;
pthread_mutex_t mutex_workers;

void cargarConfigMaster(char* nombre_config_sin_formato){
    char* path_completo = string_from_format("../configs/%s.config", nombre_config_sin_formato);

    t_config* config = config_create(path_completo);
    config_master = malloc(sizeof(ConfigMaster));

    if(config_master == NULL){
        log_error(logger_master,"(cargarConfigMaster) - Fallo malloc de la config");
        exit(EXIT_FAILURE);
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
        //int* fd_cliente = malloc(sizeof(int));
        //*fd_cliente = esperarCliente(socket,logger_master);
        int fd_nuevo = esperarCliente(socket,logger_master);

        pthread_t thread;
        int* p_fd = malloc(sizeof(int));
        *p_fd = fd_nuevo;
        
        
        pthread_create(&thread, NULL, gestionarClienteIndividual, p_fd);
        pthread_detach(thread); //creo que no hace falta, pero pensar a futuro        
    }
}

void* gestionarClienteIndividual(void* args){
    int fd_conexion = *(int *)args; //clarisimo mi rey
    free(args);

    Mensaje* handshake = recibirMensajito(fd_conexion, logger_master);

    if (!handshake) {
        log_warning(logger_master, "(gestionarClienteIndividual) - Cliente %d se desconectó sin handshake", fd_conexion);
        close(fd_conexion);
        return NULL;
    }

    log_debug(logger_master,
        "Debug - (gestionarClienteIndividual) - handshake %s",handshake->mensaje);

    char** mensajito_cortado = string_split(handshake->mensaje," ");
    int cod_op = obtenerModuloCodOp(mensajito_cortado[0]);    

    switch (cod_op){
        
    case QUERY: //0
        //QUERY nombre_query prioridad
        //--->args:nombre_query prioridad fd_cliente
        char* nombre_query = string_duplicate(mensajito_cortado[1]);
        int prioridad = atoi(mensajito_cortado[2]); 
        string_array_destroy(mensajito_cortado);
        liberarMensajito(handshake);

        gestionarQueryIndividual(nombre_query, prioridad, fd_conexion); 
        free(nombre_query);

        // Dejo socket abierto para querys
        atenderQueryControl(fd_conexion);
        break;
    case WORKER: //2

        int id_worker = atoi(mensajito_cortado[1]);
        string_array_destroy(mensajito_cortado);
        liberarMensajito(handshake);

        gestionarWorkerIndividual(id_worker,fd_conexion);

        atenderWorker(fd_conexion);
        break;
    default:
            log_error(logger_master,"Error - (gestionarClienteIndividual) - Handshake invalido desde %d", fd_conexion); //capaz un abort, quien sabe
            string_array_destroy(mensajito_cortado);
            liberarMensajito(handshake);
            close(fd_conexion);
    }
    return NULL; // tira warn sino
}

void hiloAging(void* args){
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
}


void realizarDesalojo(Worker* vistima, Query* nueva_query) {
    vistima->query_pendiente = nueva_query;

    Mensaje* msg = crearMensajito("DESALOJAR");
    enviarMensajito(msg, vistima->fd, logger_master);

    log_debug(logger_master, "DESALOJO solicitado -> Query %d espera a Worker %d",nueva_query->quid,vistima->id);
}

bool comparar_prioridad(Query* a, Query* b) {
    return a->prioridad < b->prioridad;  // menor número = mayor prioridad
}


void intentarPlanificarDesdeReady() {
    pthread_mutex_lock(&mutex_lista_ready);
    pthread_mutex_lock(&mutex_workers);

    if (list_is_empty(lista_ready)) {
        pthread_mutex_unlock(&mutex_workers);
        pthread_mutex_unlock(&mutex_lista_ready);
        return;
    }


    Query* candidata = NULL;

    if (string_equals_ignore_case(config_master->algoritmo_plani, "FIFO")) {
        candidata = list_get(lista_ready, 0);
    } else {
        list_sort(lista_ready, ordenarPorPrioridad);
        candidata = list_get(lista_ready, 0);
    }

    Worker* worker_libre = buscarWorkerLibre();
    if (worker_libre) {
        Query* a_ejecutar = list_remove(lista_ready, 0);
        asignarQueryAWorker(worker_libre, a_ejecutar);
        log_info(logger_master, "Planificación  Query %d asignada por disponibilidad (Worker %d)", 
                 a_ejecutar->quid, worker_libre->id);
    }
    else if (string_equals_ignore_case(config_master->algoritmo_plani, "PRIORIDADES")) {
        Worker* victima = buscarWorkerConMenorPrioridad();
        if (victima && victima->query_actual->prioridad > candidata->prioridad) {
            Query* a_ejecutar = list_remove(lista_ready, 0);
            log_warning(logger_master, "DESALOJO por AGING  Query %d (prio %d) saca a Query %d (prio %d)",
                        a_ejecutar->quid, a_ejecutar->prioridad,
                        victima->query_actual->quid, victima->query_actual->prioridad);
            realizarDesalojo(victima, a_ejecutar);
        }
    }

    pthread_mutex_unlock(&mutex_workers);
    pthread_mutex_unlock(&mutex_lista_ready);
}


void gestionarQueryIndividual(char *nombre_query,int prioridad,int fd){
    Query* nueva_query = crearQuery(nombre_query,prioridad,fd);
    if(!nueva_query) return;
    char* path_query = string_from_format("%s/%s",path_base_query,nombre_query);
    //////////////////////////////////////////// LOG OBLIGATORIO ////////////////////////////////////////////////////////////////
    log_info(logger_master, "Se conecta un Query Control para ejecutar la Query %s con prioridad %d - Id asignado: %d. Nivel multiprocesamiento %d",
        path_query,prioridad,nueva_query->quid, list_size(lista_workers));
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    free(path_query);


    pthread_mutex_lock(&mutex_lista_ready);
    pthread_mutex_lock(&mutex_workers);


    bool estaba_vacia = list_is_empty(lista_ready);
    int id_query_nueva = nueva_query->quid;
    list_add(lista_ready, nueva_query);
    log_info(logger_master, "Query %d llegada - Prioridad: %d", nueva_query->quid, prioridad);

    //ESTO CUBRE FIFO
    if(string_equals_ignore_case(config_master->algoritmo_plani, "FIFO")){
        if(estaba_vacia){
            Worker* worker_libre = buscarWorkerLibre();
            if (worker_libre) {
                Query* primera = list_remove(lista_ready, 0);
                if(primera->quid != id_query_nueva){
                    log_error(logger_master,"(gestionarQueryIndividual) - Incongruencia");
                    pthread_mutex_unlock(&mutex_workers);
                    pthread_mutex_unlock(&mutex_lista_ready);
                    exit(EXIT_FAILURE);
                }
                asignarQueryAWorker(worker_libre, primera);
            }
        }
    }
    else if (string_equals_ignore_case(config_master->algoritmo_plani, "PRIORIDADES")) {
        list_sort(lista_ready,ordenarPorPrioridad);

        Query* consulto_mas_prioritaria = list_get(lista_ready,0);

        if(consulto_mas_prioritaria->quid != id_query_nueva){ // No soy la query mas prioritaria en READY
            pthread_mutex_unlock(&mutex_workers);
            pthread_mutex_unlock(&mutex_lista_ready);
            return;
        } 

        // Soy la query mas prioritaria en READY
        Worker* worker_libre = buscarWorkerLibre();

        
        if (worker_libre){ 
            Query* mas_prioritaria = list_remove(lista_ready, 0); // aca realmente la saco, antes la consulte nomas
            asignarQueryAWorker(worker_libre, mas_prioritaria);
        }
        else {
            Worker* vistima = buscarVictimaDesalojable(consulto_mas_prioritaria->prioridad);

            if (vistima && vistima->query_actual->prioridad > consulto_mas_prioritaria->prioridad) {
                Query* mas_prioritaria = list_remove(lista_ready, 0); // aca realmente la saco, antes la consulte nomas

                log_debug(logger_master, "(gestionarQueryIndividual) - Pedido de DESALOJO Query %d (prio %d) saca a Query %d (prio %d)",
                            mas_prioritaria->quid, mas_prioritaria->prioridad,
                            vistima->query_actual->quid, vistima->query_actual->prioridad);
                
                realizarDesalojo(vistima, mas_prioritaria);
            }
            log_debug(logger_master,
                    "Debug - (gestionarQueryIndividual) - No se encontró worker con query ejecutando con menor prioridad que la Query candidata");    

        }
    }
    pthread_mutex_unlock(&mutex_workers);
    pthread_mutex_unlock(&mutex_lista_ready);
}




void asignarQueryAWorker(Worker* worker, Query* query) { // los mutex deberian estar tomados
    if (!worker || !query) return;

    // Marco worker como ocupado
    agarrarLaPala(worker, query);


    // Enviar la query al Worker
    char* mensaje_query = string_from_format("QUERY %d %s", query->quid, query->query);
    Mensaje* mensajito = crearMensajito(mensaje_query);
    free(mensaje_query);

    enviarMensajito(mensajito, worker->fd, logger_master);

    log_info(logger_master, "Se envía la Query %d (%d) al Worker %d", 
             query->quid, query->prioridad, worker->id);
}

//despacha si hay worker libre
void intentarEnviarQueryAExecute(Query *query_que_quiere_laburar){
    
    Worker* laburito_disponible = (Worker *)list_find(lista_workers,buscarLaburanteSinLaburo); //probar esto che
    if (laburito_disponible!=NULL){
        Mensaje *mensajito_a_despachar = crearMensajito(query_que_quiere_laburar->query);
        enviarMensajito(mensajito_a_despachar,query_que_quiere_laburar->fd,logger_master);

        laburito_disponible->query_actual = query_que_quiere_laburar; 
        laburito_disponible->esta_libre = false;
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
    list_add(lista_workers, nuevo_laburante_devuelto_por_la_funcion_que_crea_y_devuelve_worker);

    //////////////////////////////////////////// LOG OBLIGATORIO ////////////////////////////////////////////////////////////////
    log_info(logger_master, "Se conecta el Worker %d - Cantidad total de Workers: %d",id_worker, list_size(lista_workers));
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    pthread_mutex_unlock(&mutex_workers);

}

// no se reserva nunca worker

/*
prioridades: si no hay workers libres y justo nuestro worker es el de menor prioridad


*/

void intentarEnviarQueryAExecutePorWorker(Worker* worker){ // linkedin
    if(!worker) return;

    pthread_mutex_lock(&mutex_workers);
    pthread_mutex_lock(&mutex_lista_ready);

    if(worker->query_pendiente != NULL){
        Query* la_que_estaba_esperando_a_que_el_trabajador_se_desocupe = worker->query_pendiente;
        worker->query_pendiente = NULL;
        asignarQueryAWorker(worker, la_que_estaba_esperando_a_que_el_trabajador_se_desocupe);
        log_debug(logger_master, "Worker %d recibe Query %d (pendiente por DESALOJO)", worker->id, la_que_estaba_esperando_a_que_el_trabajador_se_desocupe->quid);
        pthread_mutex_unlock(&mutex_lista_ready);
        pthread_mutex_unlock(&mutex_workers);
        return;
    }

    if (!list_is_empty(lista_ready)) {
        Query* siguiente = NULL;

        if (string_equals_ignore_case(config_master->algoritmo_plani, "FIFO")) {
            siguiente = list_remove(lista_ready, 0);
        } else {
            list_sort(lista_ready, ordenarPorPrioridad); // Por las dudas tengo los mutex no me voy a hacer el piola
            siguiente = list_remove(lista_ready, 0);
        }

        asignarQueryAWorker(worker, siguiente);
        log_info(logger_master, "Worker %d recibe Query %d desde READY", worker->id, siguiente->quid);
    } else {
        worker->esta_libre = true;     
        worker->query_actual = NULL;
        log_debug(logger_master, "Worker %d queda LIBRE (sin queries)", worker->id);
        
    }
    pthread_mutex_unlock(&mutex_lista_ready);
    pthread_mutex_unlock(&mutex_workers);
}


Worker* crearWorker(int id_worker_a_crear_ahora, int contacto_empleado){
    Worker* nuevo_laburante_que_reserva_memoria_en_el_espacio_heap = malloc(sizeof(Worker));
    nuevo_laburante_que_reserva_memoria_en_el_espacio_heap->esta_libre = true;
    nuevo_laburante_que_reserva_memoria_en_el_espacio_heap->id = id_worker_a_crear_ahora;
    nuevo_laburante_que_reserva_memoria_en_el_espacio_heap->query_actual = NULL;
    nuevo_laburante_que_reserva_memoria_en_el_espacio_heap->fd = contacto_empleado;
    return nuevo_laburante_que_reserva_memoria_en_el_espacio_heap; 
} 

bool hayLaburo(t_list* lista){
    return !(list_is_empty(lista));
}

void agarrarLaPala(Worker* laburador,Query* laburo){
        laburador->query_actual = laburo;
        laburador->esta_libre = false;
}

bool buscarLaburanteSinLaburo(void *args){
    Worker* laburante = (Worker *)args;
    return laburante->esta_libre;
}

void inicializarSemaforosMaster(){
    pthread_mutex_init(&mutex_lista_ready,NULL);
    pthread_mutex_init(&mutex_quid_global,NULL);
    pthread_mutex_init(&mutex_workers,NULL);
}

void inicializarListas(){
    lista_ready = list_create();
    lista_workers = list_create();
}


void atenderWorker(int fd_worker) {
    Worker* worker = buscarWorkerPorFd(fd_worker);
    if (!worker) return;

    while (1) {
        Mensaje* msg = recibirMensajito(fd_worker, logger_master);

        if (!msg) {  // Worker murió
            log_error(logger_master, "Worker %d se desconectó inesperadamente", worker->id);
            //manejarDesconexionWorker(worker);
            break;
        }

        char** mensaje_array = string_split(msg->mensaje, " ");
        int cod_op = obtenerRespuestaWorkerEnum(mensaje_array[0]);

        switch (cod_op){
        case LEER:
            enviarMensajito(msg, worker->query_actual->fd, logger_master);
            //////////////////////////////////////////// LOG OBLIGATORIO ////////////////////////////////////////////////////////////////
            log_info(logger_master, "Se envía un mensaje de lectura de la Query %d en el Worker %d al Query Control",worker->query_actual->quid, worker->id);
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
            string_array_destroy(mensaje_array);
            continue;

            break;
        case DESALOJAR: 

            int pc = atoi(mensaje_array[1]);

            worker->query_actual->pc = pc; // actualizacion de pc

            //manejarDesalojo(worker, msg->mensaje);
            ///////////////////////////////////////////// LOG OBLIGATORIO ////////////////////////////////////////////////////////////////
            log_info(logger_master, "Se desaloja la Query %d %d del Worker %d - Motivo: PRIORIDAD",
                worker->query_actual->quid, worker->query_actual->prioridad, worker->id);
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            intentarEnviarQueryAExecutePorWorker(worker);
            break;
        case FINALIZAR:
            ///////////////////////////////////////////// LOG OBLIGATORIO ////////////////////////////////////////////////////////////////
            log_info(logger_master, "Se terminó la Query %d en el Worker %d",worker->query_actual->quid, worker->id);
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            ///////////////////////////////////////////// LOG OBLIGATORIO ////////////////////////////////////////////////////////////////
            log_info(logger_master, "Se desconecta un Query Control. Se finaliza la Query %d con prioridad %d. Nivel multiprocesamiento %d",
                worker->query_actual->quid, worker->query_actual->prioridad, list_size(lista_workers));
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


           Mensaje* mensaje_end_finaliza = crearMensajito("0 Finaliza");
           enviarMensajito(mensaje_end_finaliza, worker->query_actual->fd, logger_master);
           
            Query * query_actual = worker->query_actual;
            worker->query_actual = NULL;
            free(query_actual->query);
            free(query_actual);
           
        
            intentarEnviarQueryAExecutePorWorker(worker);
            break;

        case ERROR:

            char* motivo = mensaje_array[1];
            ///////////////////////////////////////////// LOG OBLIGATORIO ////////////////////////////////////////////////////////////////
            log_info(logger_master, "Se terminó la Query %d en el Worker %d",worker->query_actual->quid, worker->id);
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            ///////////////////////////////////////////// LOG OBLIGATORIO ////////////////////////////////////////////////////////////////
            log_info(logger_master, "Se desconecta un Query Control. Se finaliza la Query %d con prioridad %d. Nivel multiprocesamiento %d",
                worker->query_actual->quid, worker->query_actual->prioridad, list_size(lista_workers));
                
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            char* formato = string_from_format("0 %s", motivo);
            Mensaje* mensaje_error_query = crearMensajito(formato);
            free (formato);
            enviarMensajito(mensaje_error_query,worker->query_actual->fd,logger_master);
            
            Query * query_actual_d = worker->query_actual;
            worker->query_actual = NULL;
            
            free(query_actual_d->query);
            free(query_actual_d);

            
            intentarEnviarQueryAExecutePorWorker(worker);
            break;

        
        case CARTA_DOCUMENTO:

                

                int id_worker = worker->id;

                worker = list_remove(lista_workers, id_worker);
                 ///////////////////////////////////////////// LOG OBLIGATORIO ////////////////////////////////////////////////////////////////
                log_info(logger_master, "Se desconecta el Worker %d - Se finaliza la Query %d - Cantidad total de Workers: %d",
                worker->id,worker->query_actual->quid, list_size(lista_workers));
                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


                list_add(lista_ready,worker->query_actual);
                if(worker->query_pendiente != NULL)
                     list_add(lista_ready,worker->query_pendiente);
                free(worker);



        default:
            log_error(logger_master, "(atenderWorker) - Error al obtener cod_op de Worker");
            exit(EXIT_FAILURE);
            break;
        }

        string_array_destroy(mensaje_array);

        liberarMensajito(msg);
    }
}

void atenderQueryControl(int fd_qc) {
    Query* query = buscarQueryPorFd(fd_qc);
    if (!query) return;

    while (1) {
        Mensaje* msg = recibirMensajito(fd_qc, logger_master); //nunca va a pasar
        if (!msg) {
            log_warning(logger_master, "Query Control de Query %d se desconectó", query->quid);
            //cancelarQuery(query);
            break;
        }
        liberarMensajito(msg);
    }
}


Worker* buscarWorkerPorFd(int fd_buscado) {
    pthread_mutex_lock(&mutex_workers);

    for (int i = 0; i < list_size(lista_workers); i++) {
        Worker* w = list_get(lista_workers, i);
        if (w->fd == fd_buscado) {
            pthread_mutex_unlock(&mutex_workers);
            return w;
        }
    }

    pthread_mutex_unlock(&mutex_workers);
    return NULL;
}

Query* buscarQueryPorFd(int fd_buscado) {

     pthread_mutex_lock(&mutex_workers);

    for (int i = 0; i < list_size(lista_workers); i++) {
        Worker* w = list_get(lista_workers, i);
        if (w->query_actual->fd == fd_buscado) {
            pthread_mutex_unlock(&mutex_workers);
            return w->query_actual;
        }
    }
    pthread_mutex_unlock(&mutex_workers);

    pthread_mutex_lock(&mutex_lista_ready);

    for (int i = 0; i < list_size(lista_ready); i++) {
        Query* q = list_get(lista_ready, i);
        if (q->fd == fd_buscado) {
            pthread_mutex_unlock(&mutex_lista_ready);
            return q;
        }
    }
    pthread_mutex_unlock(&mutex_lista_ready);
    return NULL;
}

Query* buscarQueryPorIdListaReady(int id_buscado) {
    pthread_mutex_lock(&mutex_lista_ready);

    for (int i = 0; i < list_size(lista_ready); i++) {
        Query* q = list_get(lista_ready, i);
        if (q->quid == id_buscado) {
            pthread_mutex_unlock(&mutex_lista_ready);
            return q;
        }
    }

    pthread_mutex_unlock(&mutex_lista_ready);
    return NULL;
}

Query* buscarQueryPorIdListaWorkers(int id_buscado) {
    pthread_mutex_lock(&mutex_workers);

    for (int i = 0; i < list_size(lista_workers); i++) {
        Worker* w = list_get(lista_workers, i);
        if (w->query_actual && w->query_actual->quid == id_buscado) {
            pthread_mutex_unlock(&mutex_workers);
            return w->query_actual;
        }
    }

    pthread_mutex_unlock(&mutex_workers);
    return NULL;
}

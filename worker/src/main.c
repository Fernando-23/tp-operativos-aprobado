#include "helpers-worker.h"
#include "ciclo_instruccion.h"
#include "memoria_interna.h"
#include <unistd.h>
#include <sys/socket.h>


static void manejador_senial(int signo){
    debo_morir = 1;
}

int main(int argc, char* argv[]) {
    //signal(SIGINT, menejador_senial);
    signal(SIGINT, manejador_senial);
    signal(SIGTERM, manejador_senial);

    siginterrupt(SIGINT, 1);
    siginterrupt(SIGTERM, 1);

    char* path_config = argv[1];
    id_worker = atoi(argv[2]);

    cargarConfigWorker(path_config);

    // eliminarConexion(config_worker->ip_storage, config_worker->puerto_storage);
    // eliminarConexion(config_worker->ip_master, config_worker->puerto_master);
    error_en_operacion = string_duplicate("OK");
    logger_worker = iniciarLogger("worker", config_worker->log_level);

    inicializarMutexWorker();

    query = malloc(sizeof(Query));
    instruccion = malloc(sizeof(t_instruccion));
    pthread_mutex_init(&mutex_interrumpir_query,NULL);
    log_info(logger_worker, "Worker %d iniciado correctamente", id_worker);

    //conexion a storage que devuelve el tamanio de pagina

    //pthread_mutex_lock(&mx_conexion_storage);
    socket_storage = conexionStorage();

    if(socket_storage == -1) 
        return 1;
    //pthread_mutex_unlock(&mx_conexion_storage);
    
    // hilo interrumpir_query = true

    //conexion a master (pero el recv se hace a parte)
    //pthread_mutex_lock(&mx_conexion_master);
    conexionMaster();  
    
    //pthread_mutex_unlock(&mx_conexion_master);
    sleep(1); //nadie vio nada asereje

    conexionMasterDesalojo();

    iniciarMemoria();  //chequear despues si esta bien asignado

    pthread_t thread_desalojo;
    pthread_create(&thread_desalojo, NULL, hiloDesalojo, NULL);
    pthread_detach(thread_desalojo);

    
    while (!debo_morir) { //Exit -> libre -> checkeo_interrupt

        log_info(logger_worker,"Esperando datos de master"); 
        esperandoQuery(socket_master);
        
        while(!tengo_que_cambiar_contexto) {
            
            char* instruccion = Fetch();  // "WRITE 345 42"
    
            char** instruccion_separada = Decode(instruccion);
            log_debug(logger_worker,"Instrucción a ejecutar: %s", instruccion);

            es_error_o_end = Execute(instruccion_separada);
            if(es_error_o_end){
                log_debug(logger_worker, "Query %d: Finalizo TENGO_QUE_CAMBIAR_CONTEXTO", query->id_query);
                tengo_que_cambiar_contexto = true;
            } 
            else query->pc_query++;

            checkInterrupt(); // setea tengo que cambiar contexto a true si hay interrupcion
        }

        list_destroy_and_destroy_elements(query->instrucciones,destruir);
        gestionarCheckInterrupt();

        log_debug(logger_worker, "Se va a cambiar el contexto");
        tengo_que_cambiar_contexto = false;
        hubo_interrupcion = false;

    }

    log_info(logger_worker, "Worker recibio sigint o sigterm");
    char* mensaje_de_finalizacion;
    mensaje_de_finalizacion = string_from_format("CARTA_DOCUMENTO %d %d",query->id_query, id_worker);
    Mensaje* mensaje_storage = crearMensajito(mensaje_de_finalizacion);

    enviarMensajito(mensaje_storage, socket_storage, logger_worker);
    free(mensaje_de_finalizacion);

    mensaje_de_finalizacion = string_from_format("FINALIZAR_WORKER %d", query->pc_query);
    Mensaje* mensaje_master = crearMensajito(mensaje_de_finalizacion);
    enviarMensajito(mensaje_master, socket_master, logger_worker);

    free(mensaje_de_finalizacion);

    log_info(logger_worker, "Worker %d finalizado correctamente", id_worker);
    return 0;
}



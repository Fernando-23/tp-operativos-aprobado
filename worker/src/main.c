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

    logger_worker = iniciarLogger("worker", config_worker->log_level);

    inicializarMutexWorker();

    query = malloc(sizeof(Query));
    instruccion = malloc(sizeof(t_instruccion));
       
    log_info(logger_worker, "Worker %d iniciado correctamente", id_worker);

    //conexion a storage que devuelve el tamanio de pagina

    pthread_mutex_lock(&mx_conexion_storage);
    socket_storage = conexionStorage();

    if(socket_storage == -1) return 1;
    pthread_mutex_unlock(&mx_conexion_storage);
    
    // hilo interrumpir_query = true

    //conexion a master (pero el recv se hace a parte)
    pthread_mutex_lock(&mx_conexion_master);
    socket_master = conexionMaster();  
    if(socket_master == -1) return 1;
    pthread_mutex_unlock(&mx_conexion_master);
     
    
    iniciarMemoria();  //chequear despues si esta bien asignado


   // pthread_t th_desalojo;

    //pthread_create(&th_desalojo, NULL, hiloDesalojo, NULL);
    //pthread_detach(th_desalojo);

    //log_debug(logger_worker, "Hilo de desalojo iniciado");

    
    while (!debo_morir) { //Exit -> libre -> checkeo_interrupt

        log_debug(logger_worker,"Esperando datos de master"); 
        esperandoQuery(socket_master);
    
        while (!interrumpir_query) {
            
            char* instruccion = Fetch();  // "WRITE 345 42"
    

            char** instruccion_separada = Decode(instruccion);
            log_debug(logger_worker,"Instrucción a ejecutar: %s", instruccion);

            es_end = Execute(instruccion_separada);
    
            if(es_end){
                log_debug(logger_worker, "Query %d: Finalizo ES_END", query->id_query);
                break;
            }

            query->pc_query++; // avanzar PC como ejemplo
            
            pthread_mutex_lock(&mx_conexion_master);
            if(interrumpir_query){ // Check Interrupt
                for(int i = 0; i < list_size(tabla_general); i++){
                    TablaPaginas* tabla = list_get(tabla_general, i);
                    ejecutarFlush(tabla->file, tabla->tag);
                }
                
                log_info(logger_worker, "Query %d: Desalojada por pedido del Master", query->id_query);
                char* formato_msg_des = string_from_format("DESALOJO %d", query->pc_query);
                Mensaje* mensaje_des_master = crearMensajito(formato_msg_des);
                enviarMensajito(mensaje_des_master, socket_master, logger_worker);
                log_debug(logger_worker, "Envie mensaje de desalojo a master");
                free(formato_msg_des);

                break;
            }
            pthread_mutex_unlock(&mx_conexion_master);
        }


        list_destroy_and_destroy_elements(query->instrucciones,destruir);
        log_debug(logger_worker, "Se va a cambiar el contexto");

        interrumpir_query = true;

    }

    log_info(logger_worker, "Worker recibio sigint o sigterm");
    char* mensaje_de_finalizacion;
    mensaje_de_finalizacion = string_from_format("CARTA_DOCUMENTO %d %d",query->id_query, id_worker);
    Mensaje* mensaje_storage = crearMensajito(mensaje_de_finalizacion);

    enviarMensajito(mensaje_storage, socket_storage, logger_worker);
    free(mensaje_de_finalizacion);

    mensaje_de_finalizacion = string_from_format("FINALIZAR_WORKER");
    Mensaje* mensaje_master = crearMensajito(mensaje_de_finalizacion);
    enviarMensajito(mensaje_master, socket_master, logger_worker);

    free(mensaje_de_finalizacion);


    log_info(logger_worker, "Worker %d finalizado correctamente", id_worker);
    return 0;
}



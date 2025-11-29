#include "helpers-worker.h"
#include "ciclo_instruccion.h"
#include <stdlib.h>

t_log* logger_worker = NULL;
ConfigWorker* config_worker = NULL;

bool es_end = false;

pthread_mutex_t mx_conexion_storage;
pthread_mutex_t mx_conexion_master;
pthread_mutex_t mx_recibir_query;
pthread_mutex_t mx_bitmap;
pthread_mutex_t sem_instruccion;

Query* query;
Puntero ptr_gb;
bool interrumpir_query=false;
bool requiere_realmente_desalojo;

int tam_pag;


void inicializarMutexWorker(){
    pthread_mutex_init(&mx_conexion_storage, NULL);
    pthread_mutex_init(&mx_conexion_master, NULL);
    pthread_mutex_init(&mx_recibir_query, NULL);
    pthread_mutex_init(&mx_bitmap, NULL);
    pthread_mutex_init(&sem_instruccion, NULL);
}



void cargarConfigWorker(char* arch_config){
    char* path_completo = string_from_format("../configs/%s.config",arch_config);


    t_config* config = iniciarConfig(path_completo);
    config_worker = malloc(sizeof(ConfigWorker));
    

    if (config_worker == NULL) {
        abort(); 
    }

    config_worker->ip_master = string_duplicate(config_get_string_value(config, "IP_MASTER"));
    config_worker->puerto_master= string_duplicate(config_get_string_value(config, "PUERTO_MASTER"));
    config_worker->ip_storage = string_duplicate(config_get_string_value(config, "IP_STORAGE"));
    config_worker->puerto_storage = string_duplicate(config_get_string_value(config,"PUERTO_STORAGE"));
    config_worker->tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
    config_worker->retardo_memoria = config_get_int_value(config, "RETARDO_MEMORIA");
    config_worker->algoritmo_reemplazo = string_duplicate(config_get_string_value(config, "ALGORITMO_REEMPLAZO"));
    config_worker->path_queries = string_duplicate(config_get_string_value(config, "PATH_SCRIPTS"));
    config_worker->log_level = config_get_int_value(config, "LOG_LEVEL");

    free(path_completo);
    config_destroy(config);
}

int conexionStorage(){

     socket_storage = crearConexion(config_worker->ip_storage,config_worker->puerto_storage, logger_worker);
    if(socket_storage == -1){
        log_error(logger_worker,"(conexionStorage) - No se pudo conectar con Storage");
        return -1;
    }
    Mensaje* mensajito = crearMensajito("Riding in my storage, right after a commit."); // polemico
    enviarMensajito(mensajito,socket_storage, logger_worker);
    recv(socket_storage,&tam_pag,sizeof(int),MSG_WAITALL);
    log_info(logger_worker,"Tamanio pag recibido %d",tam_pag);
    
    log_info(logger_worker,"Conectado a Storage");

    return socket_storage;
}


int conexionMaster(){

    socket_master = crearConexion(config_worker->ip_master,config_worker->puerto_master, logger_worker);
    if(socket_master == -1){
        log_error(logger_worker,"(conexionMaster) - No se pudo conectar con Master");
        return -1;
    }
    Mensaje* mensajito = crearMensajito("hola mi estimado master, soy tu worker");
    enviarMensajito(mensajito,socket_master,logger_worker);
    
    log_info(logger_worker,"Conectado a Master");

    return socket_master;
}

void esperandoQuery(int socket){
    
    // Espero los datos de Master para continuar
    // id_query, pc_query y path_query
    pthread_mutex_lock(&mx_recibir_query);
    Mensaje* datos_query = recibirMensajito(socket, logger_worker);
    pthread_mutex_unlock(&mx_recibir_query);
    if(datos_query == NULL){
        log_error(logger_worker,"No me llegaron datos de Master");
    }
    
    char ** lista_de_datos = string_split(datos_query->mensaje," ");

    if (lista_de_datos == NULL || lista_de_datos[0] == NULL || lista_de_datos[1] == NULL || lista_de_datos[2] == NULL) {
        log_error(logger_worker, "Datos de Query incompletos");
        if (lista_de_datos) {
            for (int i = 0; lista_de_datos[i] != NULL; i++) {
                free(lista_de_datos[i]);
            }
            free(lista_de_datos);
        }
        liberarMensajito(datos_query);
        return;
    }

    query->id_query = atoi(lista_de_datos[0]);
    query->pc_query = atoi(lista_de_datos[1]);
    query->nombre = string_duplicate(lista_de_datos[2]);
    query->instrucciones = crearListaDeInstrucciones();
    log_info(logger_worker, "## Query %d : Se recibe la Query. El path de operaciones es: %s", query->id_query, query->nombre);

    for (int i = 0; lista_de_datos[i] != NULL; i++) {
        free(lista_de_datos[i]);
    }
    free(lista_de_datos);

    liberarMensajito(datos_query);
}



void* hiloDesalojo(void* args){

    while(1){
        Mensaje* msg = recibirMensajito(socket_master, logger_worker);

        if(!msg){
            log_error(logger_worker, "Master se desconecto. Terminando worker...");
            exit(EXIT_FAILURE);
        }

        if(string_equals_ignore_case(msg->mensaje, "DESALOJAR")){
            log_debug(logger_worker, "Recibido Desalojo del Master. Interrumpiendo ejecucion actual...");
            interrumpir_query = true;
            liberarMensajito(msg);
            break;
        }

        liberarMensajito(msg);

    }

    return NULL;
}

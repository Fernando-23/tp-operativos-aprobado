#include "helpers-worker.h"
#include <stdlib.h>

t_log* logger_worker = NULL;
ConfigWorker* config_worker = NULL;
Query* query;

bool interrumpir_query;
bool requiere_realmente_desalojo;

int tam_pag;



void cargarConfigWorker(char* path_config){

    char* path_completo = string_new();
    string_append(&path_completo, "../configs/");
    string_append(&path_completo, path_config);

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

int conexion_storage(){

    int socket_storage = crear_conexion(config_worker->ip_storage,config_worker->puerto_storage);
    if(socket_storage == -1){
        log_error(logger_worker,"No se pudo conectar con Storage");
        exit(EXIT_FAILURE);
    }
    Mensaje* mensajito = crearMensajito("Riding in my storage, right after a commit.");
    enviarMensajito(mensajito,socket_storage,logger_worker);
    recv(socket_storage,&tam_pag,sizeof(int),MSG_WAITALL);
    log_info(logger_worker,"Tamanio pag recibido %d",tam_pag);
    
    log_info(logger_worker,"Conectado a Storage");

    return socket_storage;
}


int conexion_master(){

    int socket_master = crear_conexion(config_worker->ip_master,config_worker->puerto_master);
    if(socket_master == -1){
        log_error(logger_worker,"No se pudo conectar con Master");
        exit(EXIT_FAILURE);
    }
    Mensaje* mensajito = crearMensajito("hola mi estimado master, soy tu worker");
    enviarMensajito(mensajito,socket_master,logger_worker);
    
    log_info(logger_worker,"Conectado a Master");

    return socket_master;
}

void esperando_query(int socket){
    
    // Espero los datos de Master para continuar
    // id_query, pc_query y path_query
    pthread_mutex_lock(&recibir_query);
    Mensaje* datos_query = recibirMensajito(socket);
    pthread_mutex_unlock(&recibir_query);
    if(datos_query == NULL){
        log_error(logger_worker,"No me llegaron datos de Master");
    }
    
    char ** lista_de_datos = string_split(datos_query->mensaje," ");
    query->id_query = atoi(lista_de_datos[0]);
    query->pc_query = atoi(lista_de_datos[1]);
    *(query->path_query) = lista_de_datos[2]; //ESTO CHEQUEAR MUUUUY DETENIDAMENTE

    log_info("## Query %d : Se recibe la Query. El path de operaciones es: %s",query->id_query, query->path_query);
    liberarMensajito(datos_query);
}



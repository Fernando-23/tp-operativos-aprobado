#include "helpers-worker.h"
#include <stdlib.h>

t_log* logger_worker = NULL;
ConfigWorker* config_worker = NULL;

void CargarConfigWorker(char* path_config){

    char* path_completo = string_new();
    string_append(&path_completo, "../configs/");
    string_append(&path_completo, path_config);

    t_config* config = IniciarConfig(path_completo);
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
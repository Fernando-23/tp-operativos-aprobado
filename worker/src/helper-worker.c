#include "helpers-worker.h"
#include <stdlib.h>

t_log* logger_worker = NULL;
ConfigWorker* config_worker = NULL;

void CargarConfigWorker(char* path_config){

    char* path_completo = string_new();
    string_append(&path_completo, "../config/");
    string_append(&path_completo, path_config);

    t_config* config = IniciarConfig(path_completo);
    config_worker = malloc(sizeof(ConfigWorker));
    
    if (config_worker == NULL) {
        abort(); 
    }

    config_worker->IP_MASTER = string_duplicate(config_get_string_value(config, "IP_MASTER"));
    config_worker->PUERTO_MASTER = config_get_int_value(config, "PUERTO_MASTER");
    config_worker->IP_STORAGE = string_duplicate(config_get_string_value(config, "IP_STORAGE"));
    config_worker->PUERTO_STORAGE = config_get_int_value(config,"PUERTO_STORAGE");
    config_worker->TAM_MEMORIA = config_get_int_value(config, "TAM_MEMORIA");
    config_worker->RETARDO_MEMORIA = config_get_int_value(config, "RETARDO_MEMORIA");
    config_worker->ALGORITMO_REEMPLAZO = string_duplicate(config_get_string_value(config, "ALGORITMO_REEMPLAZO"));
    config_worker->PATH_QUERIES = string_duplicate(config_get_string_value(config, "PATH_SCRIPTS"));
    config_worker->LOG_LEVEL = config_get_int_value(config, "LOG_LEVEL");

    free(path_completo);
    config_destroy(config);
}

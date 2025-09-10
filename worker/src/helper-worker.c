#include "helpers-worker.h"
#include <stdlib.h>

t_log* logger_worker = NULL;
ConfigWorker* config_worker = NULL;


ConfigWorker* CargarConfigWorker(char* path_config){

    t_config* config = IniciarConfig(path_config);
    ConfigWorker* nuevo_config;
    nuevo_config = malloc(sizeof(ConfigWorker));
    
    if (nuevo_config == NULL) {
        abort(); 
    }

    nuevo_config->IP_MASTER = string_duplicate(config_get_string_value(config, "IP_MASTER"));
    nuevo_config->PUERTO_MASTER = config_get_int_value(config, "PUERTO_MASTER");
    nuevo_config->IP_STORAGE = string_duplicate(config_get_string_value(config, "IP_STORAGE"));
    nuevo_config->PUERTO_STORAGE = config_get_int_value(config,"PUERTO_STORAGE");
    nuevo_config->TAM_MEMORIA = config_get_int_value(config, "TAM_MEMORIA");
    nuevo_config->RETARDO_MEMORIA = config_get_int_value(config, "RETARDO_MEMORIA");
    nuevo_config->ALGORITMO_REEMPLAZO = string_duplicate(config_get_string_value(config, "ALGORITMO_REEMPLAZO"));
    nuevo_config->PATH_QUERIES = string_duplicate(config_get_string_value(config, "PATH_SCRIPTS"));
    nuevo_config->LOG_LEVEL = config_get_int_value(config, "LOG_LEVEL");

    config_destroy(config);

    return nuevo_config;
}

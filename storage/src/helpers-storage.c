#include "helpers-storage.h"

ConfigStorage* config_storage = NULL;
t_log* logger_storage = NULL;

void CargarConfigStorage(char* path_config){
    char* path_completo = string_new();
    string_append(&path_completo, "../configs/");
    string_append(&path_completo, path_config); 

    t_config* config = config_create(path_completo);
    config_storage = malloc(sizeof(ConfigStorage));
    
    if(config_storage == NULL){
        printf("cargo mal");
        abort();
    }
    
    config_storage->puerto_escucha = string_duplicate(config_get_string_value(config, "PUERTO_ESCUCHA"));
    config_storage->fresh_start = string_duplicate(config_get_string_value(config, "FRESH_START"));
    config_storage->punto_montaje = string_duplicate(config_get_string_value(config, "PUNTO_MONTAJE"));
    config_storage->retardo_operacion = config_get_int_value(config, "RETARDO_OPERACION");
    config_storage->retardo_acceso_bloque = config_get_int_value(config, "RETARDO_ACCESO_BLOQUE");
    config_storage->log_level = config_get_int_value(config, "LOG_LEVEL");
    
    free(path_completo);
    config_destroy(config);
}
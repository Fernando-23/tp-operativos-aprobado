#include "helpers-master.h"

ConfigMaster* config_master = NULL;
t_log* logger_master = NULL;

void CargarConfigMaster(char* path_config){
   
    t_config* config = config_create(path_config);
    config_master = malloc(sizeof(ConfigMaster));
    if(config_master == NULL){
        abort();
    }
    
    
    config_master->puerto_escucha = string_duplicate(config_get_string_value(config, "PUERTO_ESCUCHA"));
    config_master->algoritmo_plani = string_duplicate(config_get_string_value(config, "ALGORITMO_PLANIFICACION"));
    config_master->tiempo_aging = config_get_int_value(config, "TIEMPO_AGING");
    config_master->log_level = config_get_int_value(config, "LOG_LEVEL");
    
    config_destroy(config);
}
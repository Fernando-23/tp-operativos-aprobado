#include <helpers-query.h>

ConfigQuery* CrearConfig(char* path_config){

    t_config* config = IniciarConfig(path_config);
    ConfigQuery* nuevo_config;
    nuevo_config = malloc(sizeof(ConfigQuery));
    
    if (nuevo_config == NULL) {
        abort();
    }

    nuevo_config->ip_master = string_duplicate(config_get_string_value(config, "IP_MASTER"));
    nuevo_config->puerto_master = string_duplicate(config_get_string_value(config, "PUERTO_MASTER"));
    nuevo_config->log_level = string_duplicate(config_get_string_value(config, "LOG_LEVEL"));
    
    config_destroy(config);
    return nuevo_config;
}
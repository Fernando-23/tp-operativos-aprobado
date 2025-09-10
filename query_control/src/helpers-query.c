#include "helpers-query.h"

ConfigQuery* config_query = NULL;
t_log* logger_query = NULL;

void CargarConfigQuery(char* path_config){
    char* path_completo = string_new();
    string_append(&path_completo, "../config/");
    string_append(&path_completo, path_config);

    t_config* config = IniciarConfig(path_completo);
    config_query = malloc(sizeof(ConfigQuery));
    
    if (config_query == NULL) {
        abort();
    }

    config_query->ip_master = string_duplicate(config_get_string_value(config, "IP_MASTER"));
    config_query->puerto_master = string_duplicate(config_get_string_value(config, "PUERTO_MASTER"));
    config_query->log_level = config_get_int_value(config, "LOG_LEVEL");
    
    free(path_completo);
    config_destroy(config);
}
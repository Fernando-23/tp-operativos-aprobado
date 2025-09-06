#ifndef HELPERS_QUERY_H_
#define HELPERS__QUERY_H_

#include <commons/log.h>
#include <commons/config.h>
#include "../utils/helpers.h"


typedef struct{
    char* ip_master;
    char* puerto_master;
    char* log_level;
}ConfigQuery;

t_log* logger_query;
ConfigQuery* config_query;

ConfigQuery* CrearConfig(char* path_config);


#endif /* HELPERS_QUERY_H_ */
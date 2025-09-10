#ifndef HELPERS_QUERY_H_
#define HELPERS__QUERY_H_

#include <commons/log.h>
#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/helpers.h"
#include "../src/conexiones.h"


typedef struct{
    char* ip_master;
    char* puerto_master;
    int log_level;
}ConfigQuery;

//Variables Globales
extern t_log* logger_query;
extern ConfigQuery* config_query;


//Funciones
void CargarConfigQuery(char* path_config);


#endif /* HELPERS_QUERY_H_ */
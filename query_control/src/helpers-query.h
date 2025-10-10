#ifndef HELPERS_QUERY_H_
#define HELPERS__QUERY_H_

#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../utils/src/utils/conexiones.h"
#include "../../utils/src/utils/helpers.h"


typedef struct{
    char* ip_master;
    char* puerto_master;
    int log_level;
}ConfigQuery;

//Variables Globales
extern t_log* logger_query;
extern ConfigQuery* config_query;


//Funciones
void cargarConfigQuery(char* path_config);

Mensaje* crearMensajeRegistroQuery(char* ruta,int prioridad);
int gestionarOrdenMaestro(Mensaje* orden_de_mi_maestro);


#endif /* HELPERS_QUERY_H_ */
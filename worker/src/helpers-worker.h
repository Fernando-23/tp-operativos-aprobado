#ifndef HELPERS_WORKER_H_
#define HELPERS__WORKER_H_

#include <commons/log.h>
#include <commons/config.h>
#include "../../utils/src/utils/conexiones.h"
#include "../../utils/src/utils/helpers.h"

// home/utnso/
typedef struct{
    char *ip_master;
    char* puerto_master;
    char *ip_storage;
    char* puerto_storage;
    int tam_memoria;
    int retardo_memoria;
    char *algoritmo_reemplazo;
    char *path_queries;
    int log_level;
}ConfigWorker;

extern t_log* logger_worker;
extern ConfigWorker* config_worker;

void CargarConfigWorker(char* path_config);
int crear_conexion(char *ip, char* puerto);

#endif /* HELPERS_WORKER_H_ */

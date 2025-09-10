#ifndef HELPERS_WORKER_H_
#define HELPERS__WORKER_H_

#include <commons/log.h>
#include <commons/config.h>
#include "../src/utils/helpers.h"

// home/utnso/
typedef struct{
    char *IP_MASTER;
    int PUERTO_MASTER;
    char *IP_STORAGE;
    int PUERTO_STORAGE;
    int TAM_MEMORIA;
    int RETARDO_MEMORIA;
    char *ALGORITMO_REEMPLAZO;
    char *PATH_QUERIES;
    int LOG_LEVEL;
}ConfigWorker;

extern t_log* logger_worker;
extern ConfigWorker* config_worker;

void CargarConfigWorker(char* path_config);

#endif /* HELPERS_WORKER_H_ */

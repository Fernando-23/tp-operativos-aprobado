#ifndef HELPERS_MASTER_H_
#define HELPERS_MASTER_H_
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <stdlib.h>
#include "../src/utils/conexiones.h"


typedef struct{
    char* puerto_escucha;
    char* algoritmo_plani;
    int tiempo_aging;
    int log_level;
}ConfigMaster;

extern ConfigMaster* config_master;
extern t_log* logger_master;

void CargarConfigMaster(char* path_config);

#endif /* HELPERS_MASTER_H_ */
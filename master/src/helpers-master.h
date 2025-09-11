#ifndef HELPERS_MASTER_H_
#define HELPERS_MASTER_H_
#include "../../utils/src/utils/conexiones.h"
#include "../../utils/src/utils/helpers.h"
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <stdlib.h>

typedef struct{
    char* puerto_escucha;
    char* algoritmo_plani;
    int tiempo_aging;
    int log_level;
}ConfigMaster;

extern ConfigMaster* config_master;
extern t_log* logger_master;

void CargarConfigMaster(char* path_config);
int esperar_cliente(int socket_servidor,t_log* logger);


#endif /* HELPERS_MASTER_H_ */
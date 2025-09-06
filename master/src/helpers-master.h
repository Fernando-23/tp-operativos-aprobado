#ifndef HELPERS_MASTER_H_
#define HELPERS_MASTER_H_
#include <commons/log.h>
#include <commons/config.h>


typedef struct{
    char* puerto_escucha;
    char* algoritmo_plani;
    int tiempo_aging;
    char* log_level;
}ConfigMaster;

ConfigMaster* config_master;
t_log* logger_master;

void CrearConfig(char* path_config);


#endif /* HELPERS_MASTER_H_ */
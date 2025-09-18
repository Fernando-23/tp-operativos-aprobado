#ifndef HELPERS_MASTER_H_
#define HELPERS_MASTER_H_
#include "../../utils/src/utils/conexiones.h"
#include "../../utils/src/utils/helpers.h"
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <stdlib.h>
#include <pthread.h>
#include "querys.h"

typedef struct{
    char* puerto_escucha;
    char* algoritmo_plani;
    int tiempo_aging;
    int log_level;
}ConfigMaster;

typedef enum {
    READY,
    EXEC
}EstadoQuery;


extern t_list* QueryPorEstado[2];
extern t_list* lista_ready;

extern ConfigMaster* config_master;
extern t_log* logger_master;
extern int quid_global;
extern const int cant_estados;
extern const int nivel_multiprocesamiento;


void CargarConfigMaster(char* path_config);
int esperar_cliente(int socket_servidor,t_log* logger);


#endif /* HELPERS_MASTER_H_ */
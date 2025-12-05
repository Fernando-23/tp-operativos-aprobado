#ifndef ESTRUCTURAS_MASTER_H_
#define ESTRUCTURAS_MASTER_H_
#include "../../utils/src/utils/conexiones.h"
#include "../../utils/src/utils/helpers.h"
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct{
    char* puerto_escucha;
    char* puerto_desalojo;
    char* algoritmo_plani;
    int tiempo_aging;
    char* log_level;
}ConfigMaster;

typedef enum {
    READY,
    EXEC
}EstadoQuery;

typedef struct{
    char* query;
    int prioridad;
    int quid;
    int pc;
    int fd;
}Query;

typedef struct{
    int id;
    bool esta_libre;
    int fd;
    int fd_desalojo;
    Query* query_actual;
    Query* query_pendiente;
}Worker;

extern t_list* lista_ready;
extern t_list* lista_workers;
extern pthread_mutex_t mutex_lista_ready;
extern pthread_mutex_t mutex_workers;

extern ConfigMaster* config_master;
extern t_log* logger_master;

extern const int cant_estados;
extern const int nivel_multiprocesamiento;

extern int quid_global;

extern pthread_mutex_t mutex_quid_global;


#endif /*ESTRUCTURAS_MASTER_H_*/
 

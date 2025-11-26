#ifndef HELPERS_WORKER_H_
#define HELPERS__WORKER_H_
#include <commons/log.h>
#include <commons/config.h>
#include <pthread.h>

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

typedef struct {
    int id_query;
    int pc_query;
    char* nombre;
    t_list* instrucciones;
}Query;


extern t_log* logger_worker;
extern ConfigWorker* config_worker;
extern Query* query;
extern int tam_pag;
extern bool interrumpir_query;
extern bool requiere_realmente_desalojo;

extern pthread_mutex_t mx_conexion_storage;
extern pthread_mutex_t mx_conexion_master;
extern pthread_mutex_t mx_recibir_query;

void cargarConfigWorker(char* path_config);
int conexionStorage();
int conexionMaster();
void esperandoQuery(int socket);

#endif /* HELPERS_WORKER_H_ */

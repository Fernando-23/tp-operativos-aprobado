#ifndef HELPERS_WORKER_H_
#define HELPERS_WORKER_H_
#include <commons/log.h>
#include <commons/config.h>
#include <pthread.h>

#include "../../utils/src/utils/conexiones.h"
#include "../../utils/src/utils/helpers.h"
#include "ciclo_instruccion.h"

// home/utnso/
typedef struct config_worker{
    char *ip_master;
    char* puerto_master;
    char *ip_storage;
    char* puerto_storage;
    int tam_memoria;
    int retardo_memoria;
    char *algoritmo_reemplazo;
    char *path_queries;
    char *log_level;
}ConfigWorker;

typedef struct query{
    int id_query;
    int pc_query;
    char* nombre;
    t_list* instrucciones;
}Query;

typedef struct pntero{
    int nro_tabla;
    int nro_entrada;
}Puntero;


extern Puntero ptr_gb;
extern t_log* logger_worker;
extern ConfigWorker* config_worker;
extern Query* query;
extern int tam_pag;
extern bool interrumpir_query;
extern bool es_end;
extern int id_worker;

extern pthread_mutex_t mx_conexion_storage;
extern pthread_mutex_t mx_conexion_master;
extern pthread_mutex_t mx_recibir_query;
extern pthread_mutex_t mx_bitmap;
extern pthread_mutex_t sem_instruccion;

void cargarConfigWorker(char* arch_config);
int conexionStorage();
int conexionMaster();
void esperandoQuery(int socket);

void* hiloDesalojo(void* args);

void inicializarMutexWorker();

#endif /* HELPERS_WORKER_H_ */

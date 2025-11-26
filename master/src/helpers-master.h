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
#include "prioridades.h"


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
    Query* query;
}Worker;


extern t_list* lista_ready;
extern t_list* lista_workers;
extern pthread_mutex_t mutex_lista_ready;
extern pthread_mutex_t mutex_workers;

extern ConfigMaster* config_master;
extern t_log* logger_master;

extern const int cant_estados;
extern const int nivel_multiprocesamiento;


void cargarConfigMaster(char* path_config);

void* atenderClientes(void*);
void* gestionarClienteIndividual(void* args);
void gestionarWorkerIndividual(int id_worker ,int fd_conexion);
void gestionarQueryIndividual(char *nombre_query,int prioridad,int fd);

void intentarEnviarQueryAExecute(Query *query_que_quiere_laburar);
void intentarEnviarQueryAExecutePorWorker(Worker* worker);

bool ordenarPorPrioridad(void *query_vigente_void,void* query_desafiante_void);
void agarrarLaPala(Worker* laburador,Query* laburo);
bool hayLaburo(t_list* lista);
bool buscarLaburanteSinLaburo(void *args);
void inicializarSemaforosMaster();
void inicializarListas();


void inicializarSemaforos();
/*
CONVENCIONES GESTION KAROL AQUINO

ORDEN MUTEX

WORKERS
--READY

*/


#endif /* HELPERS_MASTER_H_ */
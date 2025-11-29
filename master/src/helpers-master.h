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





void cargarConfigMaster(char* nombre_config_sin_formato);

void* atenderClientes(void*);


void* gestionarClienteIndividual(void* args);

void hiloAging(void* args);

void realizarDesalojo(Worker* vistima, Query* nueva_query);

void intentarPlanificarDesdeReady();

void gestionarWorkerIndividual(int id_worker ,int fd_conexion);
void gestionarQueryIndividual(char *nombre_query,int prioridad,int fd);

void asignarQueryAWorker(Worker* worker, Query* query);

void intentarEnviarQueryAExecute(Query *query_que_quiere_laburar);
void intentarEnviarQueryAExecutePorWorker(Worker* worker);

void atenderWorker(int fd_worker);
void atenderQueryControl(int fd_qc);

bool ordenarPorPrioridad(void *query_vigente_void,void* query_desafiante_void);

void agarrarLaPala(Worker* laburador,Query* laburo);
bool hayLaburo(t_list* lista);

bool buscarLaburanteSinLaburo(void *args);


void inicializarSemaforosMaster();
void inicializarListas();
Worker* buscarWorkerConMenorPrioridad();
Worker* buscarWorkerLibre();
Worker *buscarWorkerPorFd(int fd_worker);
Worker* crearWorker(int id_worker_a_crear_ahora, int contacto_empleado);


void hiloAging(void* args);
void inicializarSemaforos();


Query* buscarQueryPorFd(int fd_buscado);
Query* buscarQueryPorIdListaReady(int id_buscado);
Query* buscarQueryPorIdListaWorkers(int id_buscado);





/*
CONVENCIONES GESTION KAROL AQUINO

ORDEN MUTEX

WORKERS
--READY

*/


#endif /* HELPERS_MASTER_H_ */
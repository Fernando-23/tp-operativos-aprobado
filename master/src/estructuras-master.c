#include "estructuras-master.h"

const int nivel_multiprocesamiento = 0;

ConfigMaster* config_master = NULL;
t_log* logger_master = NULL;

t_list* lista_workers = NULL;
t_list* lista_ready = NULL;
pthread_mutex_t mutex_lista_ready;
pthread_mutex_t mutex_workers;

int quid_global = 0;
pthread_mutex_t mutex_quid_global;

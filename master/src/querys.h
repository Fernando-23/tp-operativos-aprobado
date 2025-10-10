#ifndef QUERYS_H_
#define QUERYS_H_
#include "helpers-master.h"
#include <commons/collections/list.h>
#include <stdlib.h>

extern int quid_global;

extern pthread_mutex_t mutex_quid_global;

Query* crearQuery(char* query, int prioridad,int fd);


#endif // QUERYS_H_
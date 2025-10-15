#ifndef PRIORIDADES_H_
#define PRIORIDADES_H_
#include "helpers-master.h"

Worker* buscarWorkerPorPrioridad(int prioridad_a_comparar);
void enviarDesalojo(int fd_worker);

#endif /*PRIORIDADES_H_*/
#ifndef PRIORIDADES_H_
#define PRIORIDADES_H_
#include "estructuras-master.h"
#include "helpers-master.h"

Worker* buscarVictimaDesalojable(int prioridad_nueva);
Worker* buscarWorkerConMenorPrioridad();
Worker* buscarWorkerLibre();
bool sigueEnReady(int quid_a_consultar);

#endif /*PRIORIDADES_H_*/
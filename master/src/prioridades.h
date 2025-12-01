#ifndef PRIORIDADES_H_
#define PRIORIDADES_H_
#include "helpers-master.h"

Worker* buscarVictimaDesalojable(int prioridad_nueva);
Worker* buscarWorkerConMenorPrioridad();
Worker* buscarWorkerLibre();
bool sigueEnReady(int quid_a_consultar);
void* realizarAgingIndividual(void *args);
#endif /*PRIORIDADES_H_*/
#include "memoria_interna.h"
#include "helpers-worker.h"

void* memoria;

void IniciarMemoria(tam_pag)
{
    memoria = malloc((size_t) config_worker->tam_memoria);
    int cant_frames = config_worker->tam_memoria / tam_pag;
     
}










#include "prioridades.h"



Worker* buscarWorkerLibre(){
    
    bool libre(void *ptr){
		Worker* worker = (Worker *)ptr;
		return worker->esta_libre;
	}

	return list_find(workers, libre);
}

Worker* buscarWorkerPorPrioridad(int prioridad_a_comparar){
    
    bool tieneMayorPrioridad(void* ptr){
        Worker* worker = (Worker *)ptr;
        return (worker->query->prioridad > prioridad_a_comparar);
    }
    return list_find(workers,tieneMayorPrioridad);
}

void enviarDesalojo(int fd_worker){
    Mensaje* mensaje_desalojo = crearMensajito("DESALOJO");
    
    enviarMensajito(mensaje_desalojo,fd_worker,logger_master);
}
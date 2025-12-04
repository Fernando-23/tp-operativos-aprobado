#include "utils_storage.h"
#include "operaciones.h"
#include "bitmap.h"

int main(int argc, char* argv[]) {
    chequearCantArgsPasadosPorTerminal(argc, 1);

    cargarConfigStorage(argv[1]);

    logger_storage = iniciarLogger("storage", config_storage->log_level);
    int socket_sv = iniciarServidor(config_storage->puerto_escucha,"Storage",logger_storage);
    inicializarSemaforos();
    
    if (socket_sv == -1) 
        return 1;

    log_debug(logger_storage, "llegue aca");
    // FRESH_STORAGE
    iniciarStorage();
    

    log_debug(logger_storage, "storage iniciado");
    pthread_t thread_rrhh;


    pthread_create(&thread_rrhh,NULL,recursosHumanos,(void *)&socket_sv); 

    pthread_join(thread_rrhh,NULL);

    return 0;
}



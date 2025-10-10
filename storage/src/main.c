#include "helpers-storage.h"
#include "operaciones.h"


int main(int argc, char* argv[]) {
    //chequearArgs(argc, 2);

    cargarConfigStorage(argv[1]);

    logger_storage = iniciarLogger("storage", config_storage->log_level);
    
    int socket_sv = iniciarServidor(config_storage->puerto_escucha,"Storage",logger_storage);
    // FRESH_STORAGE
    iniciarStorage();
    pthread_t thread_rrhh;

    pthread_create(&thread_rrhh,NULL,recursosHumanos,(void *)&socket_sv); 
    pthread_join(thread_rrhh,NULL);

    return 0;
}



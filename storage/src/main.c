#include "helpers-storage.h"


int main(int argc, char* argv[]) {
    //chequearArgs(argc, 2);

    cargarConfigStorage(argv[1]);
    logger_storage = iniciarLogger("storage", config_storage->log_level);
    
    int socket_sv = iniciarServidor(config_storage->puerto_escucha,"Storage",logger_storage);
    int socket_worker = esperarCliente(socket_sv,logger_storage); 
    char *handshake_worker = RecibirString(socket_worker);
    log_info(logger_storage,"%s",handshake_worker);
    int block_size = 16;

    send(socket_worker,&block_size,sizeof(int),0);

    return 0;
}



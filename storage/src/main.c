#include "helpers-storage.h"


int main(int argc, char* argv[]) {
    //ChequearArgs(argc, 2);

    CargarConfigStorage(argv[1]);
    logger_storage = IniciarLogger("storage", config_storage->log_level);
    
    int socket_sv = iniciar_servidor(config_storage->puerto_escucha,"Storage",logger_storage);
    int socket_worker = esperar_cliente(socket_sv,logger_storage); 
    char *handshake_worker = RecibirString(socket_worker);
    log_info(logger_storage,"%s",handshake_worker);
    int block_size = 16;

    send(socket_worker,&block_size,sizeof(int),0);

    return 0;
}



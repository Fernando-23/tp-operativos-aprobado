#include "helpers-master.h"

int main(int argc, char* argv[]) {
    //ChequearArgs(argc, 2);    de momento no recibe nada, para la entrega agregar config
    char* path_config = argv[1];
    CargarConfigMaster(path_config);
    logger_master = IniciarLogger("master", config_master->log_level);

    int socket_sv = iniciar_servidor(config_master->puerto_escucha,"master",logger_master);
    
    int socket_query = esperar_cliente(socket_sv,logger_master);       
    char* handshake_query = RecibirString(socket_query);

    log_info(logger_master,"Handshake con Query Control: %s",handshake_query);


    int socket_worker = esperar_cliente(socket_sv,logger_master);  
    char* handshake_worker = RecibirString(socket_worker);

    log_info(logger_master,"Handshake con Worker: %s",handshake_worker);

    EnviarString(handshake_query,socket_worker,logger_master);

    log_destroy(logger_master);
    return 0;
}

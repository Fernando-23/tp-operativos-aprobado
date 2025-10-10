#include "helpers-master.h"

int main(int argc, char* argv[]) {
    //ChequearArgs(argc, 2);    de momento no recibe nada, para la entrega agregar config
    char* path_config = argv[1];
    cargarConfigMaster(path_config);
    logger_master = iniciarLogger("master", config_master->log_level);

    pthread_t thread_adm;
    lista_ready = list_create();
    workers = list_create();


    int socket_sv = iniciarServidor(config_master->puerto_escucha,"master",logger_master);

    pthread_create(&thread_adm,NULL,atenderClientes,(void *)&socket_sv); 
      
    pthread_join(thread_adm,NULL);
    
    /*int socket_query = esperar_cliente(socket_sv,logger_master);       
    char* handshake_query = RecibirString(socket_query);

    log_info(logger_master,"Handshake con Query Control: %s",handshake_query);


    int socket_worker = esperar_cliente(socket_sv,logger_master);  
    char* handshake_worker = RecibirString(socket_worker);

    log_info(logger_master,"Handshake con Worker: %s",handshake_worker);

    EnviarString(handshake_query,socket_worker,logger_master);*/

    log_destroy(logger_master);
    return 0;
}

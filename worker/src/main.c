#include "helpers-worker.h"
int main(int argc, char* argv[]) {

    char* path_config = argv[1];
    char* id_worker = argv[2];

    CargarConfigWorker(path_config);
    logger_worker = IniciarLogger("worker", config_worker->log_level);
       
    log_info(logger_worker,"Se cargó todo correctamente");

    int socket_storage = crear_conexion(config_worker->ip_storage,config_worker->puerto_storage);
    
    EnviarString("hola mi estimado storage gg", socket_storage, logger_worker);
    int tam_pag;
    recv(socket_storage,&tam_pag,sizeof(int),MSG_WAITALL);
    log_info(logger_worker,"Tamanio pag recibido %d",tam_pag);

    int socket_master = crear_conexion(config_worker->ip_master,config_worker->puerto_master);

    EnviarString("hola mi estimado master yi", socket_master, logger_worker);

    
    
    //int hola = 10;

    return 0;
    
}

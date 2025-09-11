#include "helpers-query.h"

int main(int argc, char* argv[]) {
    
    //ChequearArgs(argc, 3);

    char* path_config = argv[1];
    char* path_arch_query = argv[2];
    //char* prioridad_arch_query = argv[3];

    CargarConfigQuery(path_config);
    logger_query = IniciarLogger("query", config_query->log_level);
    
    int socket_query = crear_conexion(config_query->ip_master,config_query->puerto_master);
    
    EnviarString(path_arch_query, socket_query, logger_query);
    
    log_destroy(logger_query);
    return 0;
}

#include "helpers-query.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    
    ChequearArgs(argc, 3);

    char* path_config = argv[0];
    char* path_arch_query = argv[1];
    char* prioridad_arch_query = argv[2];

    config_query = CrearConfig(path_config);
    logger_query = IniciarLogger("query", config_query->log_level);
    
    

    
    
    return 0;
}

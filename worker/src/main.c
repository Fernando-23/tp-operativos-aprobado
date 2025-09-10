#include "helpers-worker.h"
#include <commons/log.h>
int main(int argc, char* argv[]) {

    char* path_config = argv[1];
    //char* id_worker = argv[2];

    CargarConfigWorker(path_config);
    logger_worker = IniciarLogger("worker", config_worker->LOG_LEVEL);
    
    log_info(logger_worker,"El valor de IP_Master es %s", config_worker->IP_MASTER);
    printf("IP_MASTER:  %s",config_worker->IP_MASTER);

    return 0;


    
}

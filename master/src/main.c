#include "helpers-master.h"
#include "../src/conexiones.h"
#include "../src/helpers.h"

int main(int argc, char* argv[]) {
    //ChequearArgs(argc, 2);    de momento no recibe nada, para la entrega agregar config
    char* path_config = argv[1];
    CargarConfigMaster("master.config");
    logger_master = IniciarLogger("master", config_master->log_level);

    int socket_sv =iniciar_servidor(config_master->puerto_escucha,"master",logger_master);

    recibir_paquete(socket_sv);

    log_destroy(logger_master);
    return 0;
}

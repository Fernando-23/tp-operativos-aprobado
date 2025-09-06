#include "helpers-master.h"

int main(int argc, char* argv[]) {
    //ChequearArgs(argc, 2);    de momento no recibe nada, para la entrega agregar config
    CrearConfig("master.config");
    logger_master = IniciarLogger("master", config_master->log_level);
    return 0;
}

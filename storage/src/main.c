#include "helpers-storage.h"


int main(int argc, char* argv[]) {
    ChequearArgs(argc, 1);
    CargarConfigStorage("storage.config");
    logger_storage = IniciarLogger("storage", config_storage->log_level);
    
    return 0;
}

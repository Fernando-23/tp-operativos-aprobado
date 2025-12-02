#include "helpers-master.h"

int main(int argc, char* argv[]) {
    
    chequearCantArgsPasadosPorTerminal(argc, 1); 

    char* nombre_config = argv[1]; //pasalo sin el .config
    cargarConfigMaster(nombre_config); 
    
    logger_master = iniciarLogger("master", config_master->log_level);

    log_debug(logger_master, "(cargarConfigMaster) asignacion hecha");

    log_debug(logger_master, "config creada. PUERTO: %s, ALGORITMO: %s, TIEMPO_AGING: %d, LOG_LEVEL: %s",
    config_master->puerto_escucha,config_master->algoritmo_plani,config_master->tiempo_aging,config_master->log_level);
    
    inicializarSemaforosMaster();
    inicializarListas();

    pthread_t thread_adm;

    int socket_sv = iniciarServidor(config_master->puerto_escucha,"master",logger_master);

    if(socket_sv == -1) return 1;

    if (string_equals_ignore_case(config_master->algoritmo_plani, "PRIORIDADES") && 
        config_master->tiempo_aging > 0) {
        pthread_t th_aging;
        pthread_create(&th_aging, NULL, hiloAging, NULL);
        //pthread_detach(th_aging);
        log_info(logger_master, "Hilo AGING iniciado cada %d ms", config_master->tiempo_aging);
    }

    pthread_create(&thread_adm,NULL,atenderClientes,(void *)&socket_sv); 
      
    pthread_join(thread_adm,NULL);

    log_destroy(logger_master);
    return 0;
}

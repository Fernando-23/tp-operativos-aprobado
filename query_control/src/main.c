#include "helpers-query.h"

int main(int argc, char* argv[]) {
    
    chequearCantArgsPasadosPorTerminal(argc, 3);

    char* nombre_arch_de_config = argv[1]; // Archivo de configuración (sin extension)
    char* path_arch_query = argv[2]; // Archivo de Query
    char* prioridad = argv[3]; // Prioridad

    cargarConfigQuery(nombre_arch_de_config);

    logger_query = iniciarLogger("query", config_query->log_level);
    //log_info(logger_query,"IP_MASTER: %s, LOG_LEVEL:%d, CONFIG_QUERY:%s",config_query->ip_master,config_query->log_level,config_query->puerto_master);
    
    int socket_query = crearConexion(config_query->ip_master,config_query->puerto_master, logger_query);

    if (socket_query == -1 ) return 1;
    
    Mensaje* envio_query = crearMensajeRegistroQuery(path_arch_query, atoi(prioridad));
    enviarMensajito(envio_query,socket_query, logger_query);

    int tengo_que_seguir = 1;

	while(tengo_que_seguir){
		log_debug(logger_query,"Esperando el orden de mi maestro Master...");
        
		Mensaje* orden_de_mi_maestro = recibirMensajito(socket_query, logger_query); // aca se queda bloqueado sin espera

        if (orden_de_mi_maestro == NULL) {
            log_error(logger_query, "El Master cerró la conexión o se cayó. Terminando Query...");
            tengo_que_seguir = 0;
            break; 
        }
		tengo_que_seguir = gestionarOrdenMaestro(orden_de_mi_maestro);
		liberarMensajito(orden_de_mi_maestro);
	}

    
    log_destroy(logger_query);
    close(socket_query);
    return 0;
}

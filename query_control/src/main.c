#include "helpers-query.h"

int main(int argc, char* argv[]) {
    
    //ChequearArgs(argc, 3);

    char* path_config = argv[1];
    char* path_arch_query = argv[2];
    char* prioridad = argv[3];

    CargarConfigQuery(path_config);
    logger_query = IniciarLogger("query", config_query->log_level);
    
    int socket_query = crear_conexion(config_query->ip_master,config_query->puerto_master);
    Mensaje* envio_query = crearMensajeRegistroQuery(path_arch_query, prioridad);
    enviarMensajito(envio_query,socket_query);

    int tengo_que_seguir = 1;

	while(tengo_que_seguir){
		printf("Esperando el orden de mi maestro Master...\n");
		Mensaje* orden_de_mi_maestro = recibirMensajito(socket_query); // aca se queda bloqueado sin espera
		tengo_que_seguir = gestionarOrdenMaestro(orden_de_mi_maestro);
		liberarMensajito(orden_de_mi_maestro);
	}

    
    log_destroy(logger_query);
    close(socket_query);
    return 0;
}

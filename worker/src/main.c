#include "helpers-worker.h"
#include "ciclo_instruccion.h"
#include "memoria_interna.h"

int main(int argc, char* argv[]) {

    char* path_config = argv[1];
    char* id_worker = argv[2];

    cargarConfigWorker(path_config);

    logger_worker = IniciarLogger("worker", config_worker->log_level);

    query = malloc(sizeof(Query));
    instruccion = malloc(sizeof(t_instruccion));
    tabla_general = list_create();
       
    log_info(logger_worker,"Se cargó todo correctamente");

    //conexion a storage que devuelve el tamanio de pagina

    pthread_mutex_lock(&mx_conexion_storage);
    socket_storage = conexionStorage();
    pthread_mutex_unlock(&mx_conexion_storage);
    

    //conexion a master (pero el recv se hace a parte)
    pthread_mutex_lock(&mx_conexion_master);
    socket_master = conexionMaster();  
    pthread_mutex_unlock(&mx_conexion_master);
     
    
    iniciarMemoria();  //chequear despues si esta bien asignado
    
    while (1) {

        ("Esperando datos de master \n");
        esperandoQuery(socket_master);
    
        while (!interrumpir_query) {
            
            char* instruccion = Fetch();  // "WRITE 345 42"

            Decode(instruccion);
            log_debug(logger_worker,"Instrucción a ejecutar: %s\n", instruccion);

            requiere_realmente_desalojo = Execute();
            if(requiere_realmente_desalojo){
                log_debug("Execute terminó, requiere_desalojo=%d\n", requiere_realmente_desalojo);
            }
            query->pc_query++; // avanzar PC como ejemplo
        }
        list_destroy_and_destroy_elements(query->instrucciones,destruir);
        printf("[DEBUG] Se va a cambiar el contexto\n");

        interrumpir_query = false;

        //revisar con los pibes de master el tema de los desalojos
        ChequearSiTengoQueActualizarEnKernel(requiere_realmente_desalojo);
    }

    return 0;
    
}



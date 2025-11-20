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

    pthread_mutex_lock(&conexion_storage);
    socket_storage = conexion_storage();
    pthread_mutex_unlock(&conexion_storage);
    

    //conexion a master (pero el recv se hace a parte)
    pthread_mutex_lock(&conexion_master);
    socket_master = conexion_master();  
    pthread_mutex_unlock(&conexion_master);
     
    
    IniciarMemoria(&tam_pag);  //chequear despues si esta bien asignado
    
    while (1) {

        ("Esperando datos de master \n");
        esperando_query(socket_master);
        

        t_list* lista_de_instrucciones = crear_lista();

        while (!interrumpir_query) {
            
            char* instruccion = Fetch(lista_de_instrucciones);  // "WRITE 345 42"
           
            if (instruccion == NULL || instruccion[0] == '\0') {
                printf("[ERROR] No hay una instrucción válida para este PC.\n");
                return 1;
            }

            if (strcmp(instruccion, "TODO MAL") == 0) {
                printf("[ERROR] Instrucción inválida.\n");
                return 1;
            }


            Decode(instruccion);
            printf("Instrucción a ejecutar: %s\n", instruccion);

            requiere_realmente_desalojo = Execute();
            printf("Execute terminó, requiere_desalojo=%d\n", requiere_realmente_desalojo);

            query->pc_query++; // avanzar PC como ejemplo
        }
        list_destroy_and_destroy_elements(lista_de_instrucciones,destruir);
        printf("[DEBUG] Se va a cambiar el contexto\n");

        interrumpir_query = false;

        //revisar con los pibes de master el tema de los desalojos
        ChequearSiTengoQueActualizarEnKernel(requiere_realmente_desalojo);
    }

    return 0;
    
}



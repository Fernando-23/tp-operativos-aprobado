#include "helpers-worker.h"
#include "ciclo_instruccion.h"
#include "memoria_interna.h"
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>

static volatile sig_atomic_t debo_morir = 0;
static char *g_id_worker = NULL;

static void manejador_senial(int signo){
    debo_morir = 1;
}

volatile sig_atomic_t debo_morir = 0;

int main(int argc, char* argv[]) {
    //signal(SIGINT, menejador_senial);
    

    char* path_config = argv[1];
    char* id_worker = argv[2];

    if (id_worker) g_id_worker = strdup(id_worker);

    /* instalar handler de SIGINT/SIGTERM: solo marca debo_morir y desbloquea recv */
    signal(SIGINT, manejador_senal);
    signal(SIGTERM, manejador_senal);

    cargarConfigWorker(path_config);

    logger_worker = iniciarLogger("worker", config_worker->log_level);

    inicializarMutexWorker();

    query = malloc(sizeof(Query));
    instruccion = malloc(sizeof(t_instruccion));
    tabla_general = list_create();
       
    log_info(logger_worker,"Se cargó todo correctamente");

    //conexion a storage que devuelve el tamanio de pagina

    pthread_mutex_lock(&mx_conexion_storage);
    socket_storage = conexionStorage();

    if(socket_storage == -1) return 1;
    pthread_mutex_unlock(&mx_conexion_storage);
    

    //conexion a master (pero el recv se hace a parte)
    pthread_mutex_lock(&mx_conexion_master);
    socket_master = conexionMaster();  
    if(socket_master == -1) return 1;
    pthread_mutex_unlock(&mx_conexion_master);
     
    
    iniciarMemoria();  //chequear despues si esta bien asignado
    
    while (!debo_morir) { //Exit -> libre -> checkeo_interrupt

        log_debug(logger_worker,"Esperando datos de master"); 
        esperandoQuery(socket_master);
    
        while (!interrumpir_query) {
            
            char* instruccion = Fetch();  // "WRITE 345 42"

            Decode(instruccion);
            log_debug(logger_worker,"Instrucción a ejecutar: %s", instruccion);

            requiere_realmente_desalojo = Execute();
            if(requiere_realmente_desalojo){
                log_debug(logger_worker,"Execute terminó, requiere_desalojo=%d", query->id_query);
            }
            query->pc_query++; // avanzar PC como ejemplo
        }
        list_destroy_and_destroy_elements(query->instrucciones,destruir);
        printf("[DEBUG] Se va a cambiar el contexto\n");

        interrumpir_query = false;

        //revisar con los pibes de master el tema de los desalojos
        //ChequearSiTengoQueActualizarEnKernel(requiere_realmente_desalojo);
    }

    log_info(logger_worker, "Worker recibio sigint o sigterm");
    log_debug(logger_worker, "avisando a master y storage");//CARTA_DOCUMENTO queryid WORKER_ID
    char* mensaje_de_finalizacion = string_from_format("CARTA_DOCUMENTO %d %d",query->id_query, id_worker);
    Mensaje* mensaje_storage = crearMensajito(mensaje_de_finalizacion);
    enviarMensajito(mensaje_storage, socket_storage, logger_worker);

    Mensaje* mensaje_master = crearMensajito(mensaje_de_finalizacion);
    enviarMensajito(mensaje_master, socket_master, logger_worker);

    free(mensaje_de_finalizacion);

    return 0;
}



#include "utils_storage.h"



void handshake(int fd){
    Mensaje* mensajito_hanshake = malloc(sizeof(Mensaje));
    mensajito_hanshake->mensaje = string_itoa(datos_superblock_gb->tamanio_bloque);
    mensajito_hanshake->size = string_length(mensajito_hanshake->mensaje);
    
    enviarMensajito(mensajito_hanshake,fd,logger_storage);
}


void iniciarStorage(){
    
    cargarConfigSuperblock();
    
    if (string_equals_ignore_case(config_storage->fresh_start,"FRESH_START")){
        //ACA IRIA LA_SANGUINARIA();
       
  
    }
    bloques_fisicos_gb = list_create();
    files_gb = list_create();
    crearBloquesFisicos();
        
}

void cargarConfigStorage(char* path_config){
    char* path_completo = string_new();
    string_append(&path_completo, "../configs/");
    string_append(&path_completo, path_config); 

    t_config* config = config_create(path_completo);
    config_storage = malloc(sizeof(ConfigStorage));
    
    if(config_storage == NULL){
        printf("cargo mal");
        abort();
    }
    
    config_storage->puerto_escucha = string_duplicate(config_get_string_value(config, "PUERTO_ESCUCHA"));
    config_storage->fresh_start = string_duplicate(config_get_string_value(config, "FRESH_START"));
    config_storage->punto_montaje = string_duplicate(config_get_string_value(config, "PUNTO_MONTAJE"));
    config_storage->retardo_operacion = config_get_int_value(config, "RETARDO_OPERACION");
    config_storage->retardo_acceso_bloque = config_get_int_value(config, "RETARDO_ACCESO_BLOQUE");
    config_storage->log_level = config_get_int_value(config, "LOG_LEVEL");
    
    free(path_completo);
    config_destroy(config);
}

void cargarConfigSuperblock(){
    t_config* superblock = config_create("../configs/superblock.config");
    datos_superblock_gb = malloc(sizeof(ConfigSuperblock));

    if(superblock == NULL){
        printf("cargo mal");
        abort();
    }

    datos_superblock_gb->tamanio_fsystem = config_get_int_value(superblock,"FS_SIZE");
    datos_superblock_gb->tamanio_bloque = config_get_int_value(superblock,"BLOCK_SIZE");
    datos_superblock_gb->cant_bloques = calcularCantBloques();// 
    
    config_destroy(superblock);
}


int calcularCantBloques(){
    int cant_bloques = datos_superblock_gb->tamanio_fsystem / datos_superblock_gb->tamanio_bloque;

    if (datos_superblock_gb->tamanio_fsystem % datos_superblock_gb->tamanio_bloque != 0){
        return cant_bloques++;
    }

    return cant_bloques;
}

void crearBloquesFisicos(){

    for (int i = 0; i < datos_superblock_gb->cant_bloques; i++)
    {
        char* nombre_bloque;
        asignarNombreBloqueFisico(nombre_bloque,i);
        
        FILE* arch = fopen(nombre_bloque, "w+");
        fclose(arch);
        
        crearYAgregarBloqueFisicoIndividual(i,nombre_bloque);        
    }
    log_debug(logger_storage,"Debug - (crearBloquesFisicos) - Bloques fisicos creados");
}

int cantidadDeCaracteres(int numero){
    int cont = 1;
    while(numero >= 10){
        numero/= 10;
        cont++;
    }
    return cont;
}

void asignarNombreBloqueFisico(char* nombre_bloque,int i){
    switch (cantidadDeCaracteres(i)) {
            case 1: 
                sprintf(nombre_bloque,"block00%s.dat",i); 
                break;

            case 2:
                sprintf(nombre_bloque,"block0%s.dat",i);
                break;
            case 3:
                sprintf(nombre_bloque,"block%s.dat",i);
                break;
            default:
                log_error(logger_storage, "1000 o más bloques lcdtm");

        }
}

//sirve para inicializar el fs
void crearYAgregarBloqueFisicoIndividual(int id,char* nombre_bloque){
    BloqueFisico* bloque_fisico = malloc(sizeof(BloqueFisico));
    bloque_fisico->id_fisico = id;
    bloque_fisico->nombre = nombre_bloque;
    bloque_fisico->contador_hard_links = 0;
    
    list_add(bloques_fisicos_gb,bloque_fisico); 
    
}

void inicializarSemaforos(){
    pthread_mutex_init(&mutex_bitmap,NULL);
    pthread_mutex_init(&mutex_bloques_fisicos,NULL);
    pthread_mutex_init(&mutex_files,NULL);
}
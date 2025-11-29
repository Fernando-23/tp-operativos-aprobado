#include "utils_storage.h"




int obtenerTareaCodOperacion(char *string_codop){
    for (int i = 0; i < 8; i++)
    {
        if (strcmp(NOMBRE_CODOP_STORAGE[i],string_codop)==0)
		 return i;
    }
    return -1;
}


void handshake(int fd){
    char* mensajito_hanshake = string_from_format("%d",datos_superblock_gb->tamanio_bloque);
    Mensaje* mensajito_a_enviar = crearMensajito(mensajito_hanshake);
    free(mensajito_hanshake);
    enviarMensajito(mensajito_a_enviar,fd,logger_storage);
    Mensaje* resp_handshake = recibirMensajito(fd,logger_storage);
    pthread_mutex_lock(&mutex_cant_workers);
    cant_workers_conectados++;
    log_info(logger_storage,"“##Se conecta el Worker %s - Cantidad de Workers: %d",resp_handshake->mensaje,cant_workers_conectados);
    pthread_mutex_unlock(&mutex_cant_workers);
    liberarMensajito(resp_handshake);
}


void iniciarStorage(){
    crearDirectorio(PATH_PHYSICAL_BLOCKS);    

    cargarConfigSuperblock();
    
    if (string_equals_ignore_case(config_storage->fresh_start,"TRUE")){
        //ACA IRIA LA_SANGUINARIA();
        
        limpiarHashIndexConfig();
    }



    hash_index_config_gb = config_create(RUTA_HASH_INDEX);

    bloques_fisicos_gb = list_create();
    lista_files_gb = list_create();
    crearBloquesFisicos();

}

void limpiarHashIndexConfig(){
    t_config* aux_para_vaciar_hash_index = config_create(RUTA_AUX_FSTART_HASH_INDEX);  
    log_debug(logger_storage, "intenta limpiar hash");
    config_save_in_file(aux_para_vaciar_hash_index,RUTA_HASH_INDEX);
    config_destroy(aux_para_vaciar_hash_index);
    log_debug(logger_storage, "hash limpio");
}

void cargarConfigStorage(char* arch_config){
    char* path_completo = string_from_format("../configs/%s.config",arch_config);

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
       log_debug(logger_storage, "ruta_superblock incorrecta");
        exit(EXIT_FAILURE);
    }

    log_debug(logger_storage, "cargando config super block");

    datos_superblock_gb->tamanio_fsystem = config_get_int_value(superblock,"FS_SIZE");
    datos_superblock_gb->tamanio_bloque = config_get_int_value(superblock,"BLOCK_SIZE");
    datos_superblock_gb->cant_bloques = calcularCantBloques();

    log_debug(logger_storage, "se cargaron config super block ");
    
    config_destroy(superblock);
    log_debug(logger_storage, "se cargaron config super block config destroy");
}


int calcularCantBloques(){
    int cant_bloques = datos_superblock_gb->tamanio_fsystem / datos_superblock_gb->tamanio_bloque;

    if (datos_superblock_gb->tamanio_fsystem % datos_superblock_gb->tamanio_bloque != 0){
        cant_bloques++;
    }

    return cant_bloques;
}

void crearBloquesFisicos(){

    for (int i = 0; i < datos_superblock_gb->cant_bloques; i++){
        char nombre_fijo[32];
        snprintf(nombre_fijo, sizeof(nombre_fijo),"block%04d",i );
            
        char* ruta_absoluta = string_from_format("%s/%s.dat",PATH_PHYSICAL_BLOCKS,nombre_fijo);
        FILE* arch = fopen(ruta_absoluta, "a+");

        fclose(arch);
        
        
        crearYAgregarBloqueFisicoIndividual(i,string_duplicate(nombre_fijo),ruta_absoluta);
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



//sirve para inicializar el fs
void crearYAgregarBloqueFisicoIndividual(int id,char* nombre_bloque, char* ruta_absoluta){//JORGE EL CURIOSO
    BloqueFisico* bloque_fisico = malloc(sizeof(BloqueFisico));
    bloque_fisico->id_fisico = id;
    bloque_fisico->nombre = nombre_bloque;
    bloque_fisico->ruta_absoluta = ruta_absoluta;
    
    list_add(bloques_fisicos_gb,bloque_fisico); 
}

void inicializarSemaforos(){
    pthread_mutex_init(&mutex_bitmap,NULL);
     pthread_mutex_init(&mutex_files,NULL);
    pthread_mutex_init(&mutex_bloques_fisicos,NULL);
    pthread_mutex_init(&mutex_cant_workers,NULL);
   
}


Mensaje* mensajitoResultadoStorage(ErrorStorageEnum cod_error){
    Mensaje* mensajito = malloc(sizeof(Mensaje));
	
    mensajito->mensaje = string_from_format("%s", NOMBRE_ERRORES[cod_error]); // string_equals desde worker
    mensajito->size = string_length(mensajito->mensaje);
	
    return mensajito;
}

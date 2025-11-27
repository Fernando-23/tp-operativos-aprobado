#include "utils_storage.h"




int obtenerTareaCodOperacion(char *string_modulo){
    for (int i = 0; i < CANT_ERRORES; i++)
    {
        if (strcmp(NOMBRE_ERRORES[i],string_modulo)==0)
		 return i;
    }
    return -1;
}









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
        
        limpiarHashIndexConfig();
    }

    hash_index_config_gb = config_create(RUTA_HASH_INDEX);

    bloques_fisicos_gb = list_create();
    lista_files_gb = list_create();
    crearBloquesFisicos();

}

void limpiarHashIndexConfig(){
    t_config* aux_para_vaciar_hash_index = config_create(RUTA_AUX_FSTART_HASH_INDEX);
    config_save_in_file(aux_para_vaciar_hash_index,RUTA_HASH_INDEX);
    config_destroy(aux_para_vaciar_hash_index);
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

    for (int i = 0; i < datos_superblock_gb->cant_bloques; i++){
        char* nombre_bloque;
        sprintf(nombre_bloque, "block%04d",i);

        char* ruta_absoluta;
        sprintf(ruta_absoluta,"%s/%s.dat",PATH_PHYSICAL_BLOCKS,nombre_bloque);

        FILE* arch = fopen(ruta_absoluta, "a+");
        fclose(arch);
        
        crearYAgregarBloqueFisicoIndividual(i,nombre_bloque,ruta_absoluta);        
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
void crearYAgregarBloqueFisicoIndividual(int id,char* nombre_bloque, char* ruta_absoluta){//JORGE EL CURIOSO
    BloqueFisico* bloque_fisico = malloc(sizeof(BloqueFisico));
    bloque_fisico->id_fisico = id;
    bloque_fisico->nombre = nombre_bloque;
    bloque_fisico->ruta_absoluta = ruta_absoluta;
    
    list_add(bloques_fisicos_gb,bloque_fisico); 
}

void inicializarSemaforos(){
    pthread_mutex_init(&mutex_bitmap,NULL);
    pthread_mutex_init(&mutex_bloques_fisicos,NULL);
    pthread_mutex_init(&mutex_files,NULL);
}

Mensaje* mensajitoError(ErrorStorageEnum cod_error){
    Mensaje* mensajito = malloc(sizeof(Mensaje));
	
    mensajito->mensaje = string_from_format("ERROR %s", NOMBRE_ERRORES[cod_error]);
    mensajito->size = string_length(mensajito->mensaje);
	
    return mensajito;
}

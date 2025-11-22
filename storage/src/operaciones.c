#include "operaciones.h"



void* recursosHumanos(void* args_sin_formato){
    int socket_cliente = *(int *)args_sin_formato;
    
    while(1){
        int fd_cliente;
        fd_cliente = esperarCliente(socket_cliente,logger_storage);
        
        pthread_t thread_labubu;
        pthread_create(&thread_labubu, NULL, atenderLaburanteDisconforme, (void *)&fd_cliente);
        pthread_detach(thread_labubu);
    } // n veces porque hay n workers
}

void *atenderLaburanteDisconforme(void* args_sin_formato){
    int fd_cliente = *((int *)args_sin_formato);

    handshake(fd_cliente);
    
    do{
        pedidoDeLaburante(fd_cliente);
    }while(1);
}

void pedidoDeLaburante(int mail_laburante){
    
    Mensaje* mensajito;
    mensajito = recibirMensajito(mail_laburante);
      
    log_debug(logger_storage,
        "Debug - (pedidoDeLaburante) - Recibi el mensaje %s",mensajito->mensaje);

    // COD_OP QUERY_ID ...
    char** mensajito_cortado = string_split(mensajito->mensaje," ");
    CodOperacionStorage tipo_operacion = obtenerTareaCodOperacion(mensajito_cortado[0]);
    char* query_id = mensajito_cortado[1];
    hacerRetardoOperacion();
    
    switch (tipo_operacion){ 
        
    case CREATE:
    //CREATE QUERY_ID NOMBRE_FILE TAG
        char* nombre_file = mensajito_cortado[2];
        char* tag = mensajito_cortado[3];
        
        if (!realizarCREATE(query_id,nombre_file,tag)){
            enviarMensajito(mensajitoError(FILE_PREEXISTENTE),mail_laburante,logger_storage);
            log_error(logger_storage,"NO SE PUDO REALIZAR CREATE POR MOTIVO FILE_PREEXISTENTE"); // JOE PINO 
        }
        string_array_destroy(mensajito_cortado);
        break;

    case TRUNCATE:
    // TRUNCATE QUERY_ID FILE:TAG TAMANIO
        //----FILE:TAG
        char* nombre_full_file_truncate = mensajito_cortado[2];
        int tamanio_a_truncar = atoi(mensajito_cortado[3]);\

        int resultado = realizarTRUNCATE(query_id,nombre_full_file_truncate,tamanio_a_truncar);
        if (resultado != -1){
            enviarMensajito(mensajitoError(resultado),mail_laburante,logger_storage);
            log_error(logger_storage,"NO SE PUDO REALIZAR TRUNCATE POR MOTIVO %s",NOMBRE_ERRORES[resultado]); // JOE PINO 
        }

        string_array_destroy(mensajito_cortado);
        break;

    case TAG:
    //TAG QUERY_D FILE_O:TAG_O FILE_D:TAG_D
        char* nombre_completo_origen = mensajito_cortado[2];
        char* nombre_completo_destino = mensajito_cortado[3];
        enviarMensajito(mensajitoOk(), mail_laburante, logger_storage);
        
        int resultado_tag = realizarTAG(query_id,nombre_completo_origen,nombre_completo_destino);
        string_array_destroy(mensajito_cortado); 
        
        break;

    case COMMIT:
        enviarMensajito(mensajitoOk(), mail_laburante, logger_storage);
        
        string_array_destroy(mensajito_cortado);
        
        break;

    case FLUSH:
        enviarMensajito(mensajitoOk(), mail_laburante, logger_storage);
        
        string_array_destroy(mensajito_cortado);
        break;

    case DELETE:
        enviarMensajito(mensajitoOk(), mail_laburante, logger_storage);
        
        string_array_destroy(mensajito_cortado);
        break;
        
    case LEER_BLOQUE:
        enviarMensajito(mensajitoOk(), mail_laburante, logger_storage);
        
        string_array_destroy(mensajito_cortado);
        break;
    
    case ACTUALIZAR_FRAME_MODIFICADO:
        enviarMensajito(mensajitoOk(), mail_laburante, logger_storage);
        
        string_array_destroy(mensajito_cortado);
        break;

    case ERROR:
    
        enviarMensajito(mensajitoOk(), mail_laburante, logger_storage);
        
        string_array_destroy(mensajito_cortado);
        break;
    
    default:
        log_error(logger_storage,"Error - (pedidoDeLaburante) - Codigo de operacion erroneo"); //capaz un abort, quien sabe
        string_array_destroy(mensajito_cortado);
        break;
    }
    
    liberarMensajito(mensajito);

}


/*
CREATE
Esta operación creará un nuevo File dentro del FS. 
Para ello recibirá el nombre del File y un Tag inicial para crearlo.
Deberá crear el archivo de metadata en estado WORK_IN_PROGRESS y no asignarle ningún bloque.
*/

bool realizarCREATE(char* query_id,char* nombre_file, char* nombre_tag){
    
    
    if (filePreexistente(nombre_file)) return false; //chequeo de error del enunciado

    
    File* nuevo_file = crearFile(nombre_file);
    Tag* nuevo_tag = crearTag(nombre_tag,nombre_file);
    
    list_add(nuevo_file->tags, nuevo_tag);

    log_info(
        logger_storage,"## %s - File Creado %s:%s",
        query_id,nombre_file,nombre_tag);
        
    return true;
}

int realizarTRUNCATE(int query_id,char* file_completo,int tamanio_a_truncar){
    char* nombre_file; 
    char* nombre_tag;
    
    //probar funcionamiento
    asignarFileTagAChars(nombre_file,nombre_tag,file_completo);
    
    // chequeo errores
    pthread_mutex_lock(&mutex_files);
    if (fileInexistente(nombre_file)){
        free(nombre_file);
        free(nombre_tag);
        pthread_mutex_unlock(&mutex_files);
        return FILE_INEXISTENTE;
    } 

    File* file = buscarFilePorNombre(nombre_file);
    if (tagInexistente(file->tags,nombre_tag,nombre_file)){
        free(nombre_file);
        free(nombre_tag);
        pthread_mutex_unlock(&mutex_files);
        return TAG_INEXISTENTE;
    }
    
    Tag* tag_concreto = buscarTagPorNombre(file->tags,nombre_tag);
    
    gestionarTruncateSegunTamanio(tag_concreto,tamanio_a_truncar);
    actualizarTamanioMetadata(nombre_file, tag_concreto,tamanio_a_truncar);

    pthread_mutex_unlock(&mutex_files);
    free(nombre_file);
    free(nombre_tag);
    return true;
}

void hacerRetardoOperacion(){
    sleep(config_storage->retardo_operacion/1000); //te tengo al lado y me siento solo el miedo me come y no entiendo como raszones no faltan para que me
}


void actualizarTamanioMetadata(char* nombre_file, Tag* tag,int tamanio_a_truncar){
    t_config* metadata_a_actualizar = tag->metadata_config_tag;
    config_set_value(metadata_a_actualizar,"TAMANIO",string_itoa(tamanio_a_truncar));
    config_save(metadata_a_actualizar);
    log_debug(logger_storage,"Debug - (actualizarTamanioMetadata) - Actualizo metadata-File:%s - Tag: %s - Tamanio: %d",
         nombre_file, tag->nombre_tag, tamanio_a_truncar);
}


void crearDirectorio(char* path_directorio){
    struct stat st = {0};
    if (stat(path_directorio,&st) == -1){
        mkdir(path_directorio, 0777);
    }
}

File* crearFile(char* nombre_file){
    File* nuevo_file = malloc(sizeof(File));
    nuevo_file->nombre_file = nombre_file;
    char* dir_a_crear;
    sprintf(dir_a_crear,"%s/%s",RUTA_FILES,nuevo_file->nombre_file); //asignas la ruta abs del file

    crearDirectorio(dir_a_crear); //mkdir como Agustin Coda
    
    nuevo_file->tags = list_create();
}

Tag* crearTag(char* nombre_tag, char* nombre_file_asociado){
    Tag* tag = malloc(sizeof(Tag));

    tag->nombre_tag = nombre_tag;

    // Creo carpeta tag
    sprintf(tag->directorio,"%s/%s/%s",RUTA_FILES,nombre_file_asociado,tag->nombre_tag);
    crearDirectorio(tag->directorio);

    // Creo carpeta logical blocks
    char* ruta_aux_lblocks;
    sprintf(ruta_aux_lblocks,"%s/logical_blocks",tag->directorio);
    crearDirectorio(ruta_aux_lblocks);
    
    tag->metadata_config_tag = crearMetadata(tag->directorio);

    tag->bloques_logicos = list_create(); 
 
    log_debug(logger_storage,"Debug - (crearTag) - Se creo el tag %s", nombre_tag);
    
    return tag;
}

t_config* crearMetadata(char* path_tag){
    char* path_metadata;
    sprintf(path_metadata,"%s/metadata.config",path_tag);

    FILE* arch_config = fopen(path_metadata,"w+");
    fclose(arch_config);
    
    t_config* metadata = config_create(path_metadata);
    config_set_value(metadata,"TAMANIO","0");
    config_set_value(metadata,"BLOCKS","[]");
    config_set_value(metadata,"ESTADO","WORK_IN_PROGRESS");
    
    config_save_in_file(metadata,path_metadata);
    return metadata;
}




//tenes que hacer free nombre_file y tag
void asignarFileTagAChars(char* nombre_file,char* tag,char* file_a_cortar){
    char** file_cortado = string_split(file_a_cortar,":"); //FILE:TAG
 
    nombre_file = string_duplicate(file_cortado[0]);
    tag = string_duplicate(file_cortado[1]);

    string_array_destroy(file_cortado);
}



File* buscarFilePorNombre(char* nombre){
    
    bool tieneMismoNombre(void* ptr){
        File* file = (File *)ptr;
        return (strcmp(nombre,file->nombre_file));
    }
    return list_find(files_gb,tieneMismoNombre);
}

Tag* buscarTagPorNombre(t_list* tags,char* nombre_tag){
    
    bool tieneMismoNombreTag(void* ptr){
        Tag* tag = (Tag *)ptr;
        return (strcmp(nombre_tag,tag->nombre_tag));
    }

    return list_find(tags,tieneMismoNombreTag);
}

void gestionarTruncateSegunTamanio(Tag* tag_concreto, int tamanio_a_truncar){
    int tamanio_actual = config_get_int_value(tag_concreto->metadata_config_tag,"TAMANIO");
    
    if(tamanio_actual == tamanio_a_truncar){
        log_debug(logger_storage, "DEBUG -(gestionarTruncateSegunTamanio)- Mismo nuevo tamanio recibido en TRUNCATE");
        return true;
    } else if (tamanio_actual > tamanio_a_truncar){
        ferConLaMexicana(tag_concreto, tamanio_actual, tamanio_a_truncar); // siempre te podes achicar, sino miralo a Fer
        return true;
    } else {
        return agrandarEnTruncate(tag_concreto, tamanio_actual, tamanio_a_truncar);
    }
}


void ferConLaMexicana(Tag* tag, int tamanio_actual,int nuevo_tamanio){ // tag tamanio 
    int cant_bloques_a_desasignar = (tamanio_actual - nuevo_tamanio)/ datos_superblock_gb->tamanio_bloque;
    unlinkearBloquesLogicos(cant_bloques_a_desasignar, tag->bloques_logicos);
}

void unlinkearBloquesLogicos(int cant_a_unlinkear, t_list* bloques_logicos){
    for(int i = 0; i < cant_a_unlinkear; i++){
        BloqueLogico* bloque_popeado = (BloqueLogico* )list_remove(bloques_logicos,list_size(bloques_logicos));
        BloqueFisico* bloque_fisico_asociado = bloque_popeado->ptr_bloque_fisico;
        
        //---------------------------------- /files/tag/logical-blocks/0002.dat
        unlink(bloque_popeado->directorio); // quitarle el hlink
        bloque_popeado->ptr_bloque_fisico = NULL; // JORGE EL CURIOSO - Capaz por enunciado deberia apuntar al block0; 


        if (!tieneHLinks(bloque_fisico_asociado->ruta_absoluta)){ // 1 hard links -> liberar en el bitmap 
            liberarBloqueDeBitmap(bloque_fisico_asociado->id_fisico);
            log_debug(logger_storage,"(unlinkearBloquesLogicos)- El bloque fisico %s fue liberado",bloque_fisico_asociado->nombre);
        }

        liberarBloqueLogico(bloque_popeado); 
        log_debug(logger_storage,"(unlinkearBloquesLogicos) - FER lo hizo de nuevo");
    }
}



 
bool agrandarEnTruncate(Tag* tag, int tamanio_acutal,int nuevo_tamanio){
    int tamanio_a_agrandar = nuevo_tamanio - tamanio_acutal;
    int cant_bloques_necesarios = tamanio_a_agrandar / datos_superblock_gb->tamanio_bloque;
    
    asignarBloquesFisicosATag(tag,cant_bloques_necesarios);
    // actualizar metadata
    config_set_value(tag->metadata_config_tag,"TAMANIO",itoa(nuevo_tamanio));
    config_save(tag->metadata_config_tag);

    return true;
}


void asignarBloquesFisicosATagEnTruncate(Tag* tag_a_asignar_hardlinks,int cant_bloques_necesarios){     
    BloqueFisico* block0 = (BloqueFisico *)list_get(bloques_fisicos_gb,0); //esto puede ser global (block0)

    t_list* logicos_a_asignar = tag_a_asignar_hardlinks->bloques_logicos;
    int cant_bloques_antes_de_asignacion = list_size(logicos_a_asignar); // 5 8 3
    
    char** bloques_logicos = config_get_array_value(tag_a_asignar_hardlinks->metadata_config_tag,"BLOCKS");

    for (int i =  0; i < cant_bloques_necesarios; i++){
        BloqueLogico* logico_a_crear = crearBloqueLogico(cant_bloques_antes_de_asignacion);
        
        string_array_push(&bloques_logicos,string_duplicate("0"));

        list_add(logicos_a_asignar,logico_a_crear);
        cant_bloques_antes_de_asignacion++;
    }

    char *nueva_info_blocks_metadata = stringArrayConfigAString(bloques_logicos);

    config_set_value(tag_a_asignar_hardlinks->metadata_config_tag, "BLOCKS", nueva_info_blocks_metadata);
    config_save(tag_a_asignar_hardlinks->metadata_config_tag);

    free(nueva_info_blocks_metadata);
    string_array_destroy(bloques_logicos);
}

// TODO - PROXIMAMENTE EN DBZ - arreglar bien tema directorios en bloque logico y tag
BloqueLogico* crearBloqueLogico(int nro_bloque_logico,char* directorio_tag ,BloqueFisico* bloque_fisico_a_asignar){ //-- ta checkkkk  -- update 22/11/25 que hijo de puta, firma Joe 
    BloqueLogico* bloque_logico = malloc(sizeof(BloqueLogico));
    bloque_logico->ptr_bloque_fisico = bloque_fisico_a_asignar;
    bloque_logico->id_logico = nro_bloque_logico;
    sprintf(bloque_logico->directorio,"%s/logical_blocks"); 
    crearArchBloqueLogico(nro_bloque_logico, bloque_logico->directorio, bloque_fisico_a_asignar->id_fisico); 
    return bloque_logico;
}

// ESTA CHECKKKK
void crearArchBloqueLogico(int nro_bloque_logico,char* path_directorio_logico, int nro_bloque_fisico){
    char* full_path_logico;
    char* full_path_fisico;
    //----------------- fisico ------------------
    //----------------- block0 ------------------
    sprintf(full_path_fisico,"%sblock%04d.dat",PATH_PHYSICAL_BLOCKS,nro_bloque_fisico);
    
    // ---------------- logico -----------------
    sprintf(full_path_logico,"%s/%04s.dat",path_directorio_logico, nro_bloque_logico);
    
    FILE* dat_a_crear = fopen(full_path_logico,"a+"); //logical block a crear
    fclose(dat_a_crear);
    
    if(!link(full_path_fisico,full_path_logico)){
        log_error(logger_storage,"(crearArchBloqueLogico) - Error al hacer link :(");
        abort();
    }

    log_debug(
        logger_storage,"(crearArchBloqueLogico) - Bloque %d del directorio %s correctamente creado",nro_bloque_logico,path_directorio_logico);
    
    return;
}


void eliminarRespuestaConsultaBitmap(RespuestaConsultaBitmap* response_a_limpiar){
    string_array_destroy(response_a_limpiar->bloques_encontrados);
    free(response_a_limpiar);
}

void limpiarBitsPorStringArray(char** bloques_a_limpiar){
    int tope_array = string_array_size(bloques_a_limpiar);
    
    for (int i = 0; i < tope_array; i++){
        char* bloque_popeado = string_array_pop(bloques_a_limpiar);
        int nro_bloque = atoi(bloque_popeado);
        free(bloque_popeado);
        bitarray_clean_bit(bitmap_gb,nro_bloque);
    }
    log_debug(logger_storage, "Debug - (limpiarBitsPorStringArray) - Se liberaron bloques del bitmap");
}

void liberarBloqueDeBitmap(int nro_bloque){ 
     
    bitarray_clean_bit(bitmap_gb,nro_bloque);
    log_debug(logger_storage, "Debug - (limpiarBitsPorStringArray) - Se libero del bitmap el bloque nro %d",nro_bloque);
}


char* obtenerNombreBloqueConCeros(int numero){
    char* nombre;
    //NANO
    sprintf(nombre, "%04d", numero);  
    return nombre;
}

void liberarBloqueLogico(BloqueLogico* bloque_a_liberar){
    //free(bloque_a_liberar->directorio);
    //free(bloque_a_liberar->nombre);
    free(bloque_a_liberar); 
}

bool tieneHLinks(char* ruta_abs_a_consultar){
    struct stat info;
    if (stat(ruta_abs_a_consultar, &info) == -1) {
        log_debug(logger_storage,"Debug -(contarHLinks)- Error en stat");
        abort(); 
    }

    return (info.st_nlink > 1);    
}

//LIBERAR STRING RETORNADO CON CoFREE con Leche
char* stringArrayConfigAString(char** array_a_pasar_a_string){
    char* string_a_retornar = string_new();
    string_append(&string_a_retornar,"[");

    for (int i = 0; i < string_array_size(array_a_pasar_a_string); i++){
        
        string_append(&string_a_retornar,array_a_pasar_a_string[i]);

        if (i != string_array_size(array_a_pasar_a_string)-1){
            string_append(&string_a_retornar,",");
        }
    }
    string_append(&string_a_retornar,"]");

    return string_a_retornar;
}


int realizarTAG(int query_id,char* nombre_origen_completo,char* nombre_destino_completo){
    
    char* nombre_file_origen;
    char* tag_origen;
    
    char* nombre_file_destino;
    char* tag_destino;
    
    //probar funcionamiento
    asignarFileTagAChars(nombre_file_origen,tag_origen, nombre_origen_completo); //TODO

    // chequeo errores
    pthread_mutex_lock(&mutex_files);
    if (fileInexistente(nombre_file_origen)){
        free(nombre_file_origen);
        free(tag_origen);
        pthread_mutex_unlock(&mutex_files);
        return FILE_INEXISTENTE;
    } 

    File* file_origen = buscarFilePorNombre(nombre_file_origen);
    if (tagInexistente(file_origen->tags,tag_origen,nombre_file_origen)){
        free(nombre_file_origen);
        free(tag_origen);
        pthread_mutex_unlock(&mutex_files);
        return TAG_INEXISTENTE;
    }

    asignarFileTagAChars(nombre_file_destino,tag_destino, nombre_destino_completo);
    
    // file creado 
    // file tag creado
    // ninguno creado


    //TODO LO DE ABAJO ES RECONTRA INCHEQUEABLE, FIRMA JOE PINO
    //REVISA SI FILE DESTINO SE TIENE QUE CHEQUEAR POR PREEXISTENTE - ERRORES RAROS A CHECKEAR DSP 
    if (filePreexistente(nombre_file_destino)){

        pthread_mutex_unlock(&mutex_files);
        free(nombre_file_origen);
        free(tag_origen);
        free(nombre_file_destino);
        free(tag_destino);
        return FILE_PREEXISTENTE;
    }

    File* file_destino = buscarFilePorNombre(nombre_file_destino);
    if (tagPreexistente(file_destino->tags,tag_destino,nombre_file_destino)){
        pthread_mutex_unlock(&mutex_files);
        return TAG_PREEXISTENTE;
    }
    
    Tag* nuevo_tag_origen = buscarTagPorNombre(file_origen->tags,tag_origen);

    //File* nuevo_file_destino = crearFile(nombre_file);
    //Tag* nuevo_tag = crearTag(nombre_tag,nombre_file);
    
   // list_add(nuevo_file->tags, nuevo_tag);

   //log_info(
    //    logger_storage,"## %s - File Creado %s:%s",
    //    query_id,nombre_file,nombre_tag);
        
    return true;
}

int copiarTag(Tag* tag_origen,Tag* tag_destino){

    char* aux_path_metadata_destino;
    //------------------------------- PROBAR  SI REALMENTE LO COPIA EN EL DESTINO
    sprintf(aux_path_metadata_destino,"%s/metadata.config",tag_destino->directorio);
    
    config_save_in_file(tag_origen->metadata_config_tag,aux_path_metadata_destino);
    config_set_value(tag_destino,"ESTADO","WORK_IN_PROGRESS");

    //------------------------------- tag_destino ---------------> [10,2,0,0,0,0]
}

void asignarBloquesFisicosATagCopiado(Tag* tag_destino){     
    char** bloques_logicos_a_copiar = config_get_array_value(tag_destino->metadata_config_tag,"BLOCKS"); 
    t_list* logicos_a_copiar = tag_destino->bloques_logicos;
    
    if (!list_is_empty(logicos_a_copiar)){ // caso donde el tag estaba creado y CAPAZ tenia blogicos asignados
        log_debug(logger_storage,"(asignarBloquesFisicosATagCopiado) - El tag %s destino tenia cosas, se liberaron");
        unlinkearBloquesLogicos(list_size(logicos_a_copiar),logicos_a_copiar);
    }

    for (int i = 0; i < string_array_size(bloques_logicos_a_copiar); i++){
        int nro_bloque_fisico = atoi(bloques_logicos_a_copiar[i]);
        BloqueFisico* bloque_a_asignar = obtenerBloqueFisico(nro_bloque_fisico);
        BloqueLogico* logico_a_crear = crearBloqueLogico(i, bloque_a_asignar);

        list_add(logicos_a_copiar,logico_a_crear);
    }

    log_debug(logger_storage,"(asignarBloquesFisicosATagCopiado) - Bloques");
    string_array_destroy(bloques_logicos_a_copiar);
}

BloqueFisico* obtenerBloqueFisico(int nro_bloque_a_buscar){
    return (BloqueFisico *)list_get(bloques_fisicos_gb,nro_bloque_a_buscar);
}
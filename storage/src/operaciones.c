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
        //CREATE QUERY_ID NOMBRE_FILE TAG
    case CREATE:
        char* nombre_file = mensajito_cortado[2];
        char* tag = mensajito_cortado[3];
        
        bool ta_todo_bien = realizarCREATE(query_id,nombre_file,tag);
        //joe pino
        if (!ta_todo_bien){
            enviarMensajito(mensajitoError(),mail_laburante,logger_storage);
            string_array_destroy(mensajito_cortado);
            log_error(logger_storage,"{[^&^]}"); // JOE PINO 
            return;
        }
        string_array_destroy(mensajito_cortado);
        
        break;
    case TRUNCATE:
    // TRUNCATE QUERY_ID FILE:TAG TAMANIO
        char* nombre_full_file_truncate = mensajito_cortado[2];
        int tamanio_a_truncar = atoi(mensajito_cortado[3]);
        realizarTRUNCATE(query_id,nombre_full_file_truncate,tamanio_a_truncar);
        hacerRetardo();

        string_array_destroy(mensajito_cortado);
        
        break;

    case TAG:
        enviarMensajito(mensajitoOk(), mail_laburante, logger_storage);
        
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
    char* path_archivo;
    char* path_tag;
    
    char* query_id = "2"; //a cambiar en un futuro
    sprintf(path_archivo,"/utnso/home/%s",nombre_file);
    sprintf(path_tag,"%s/%s",path_archivo, nombre_tag);
    
    File* nuevo_file = crearFile(nombre_file,path_archivo);
    Tag* nuevo_tag = crearTag(nombre_tag,path_tag);
    
    list_add(nuevo_file->tags, nuevo_tag);

    log_info(
        logger_storage,"## %s - File Creado %s:%s",
        query_id,nombre_file,nombre_tag);

}

void realizarTRUNCATE(int query_id,char* file_completo,int tamanio_a_truncar){
    char* nombre_file; 
    char* nombre_tag;
    //probar funcionamiento
    asignarFileTagAChars(nombre_file,nombre_tag,file_completo);

    pthread_mutex_lock(&mutex_files);
    File* file = buscarFilePorNombre(nombre_file);
    if (file == NULL){
        // errno()
        //gestionar mejor el error
        pthread_mutex_unlock(&mutex_files);
        return;
    }

    Tag* tag_concreto = buscarTagPorNombre(file->tags,nombre_tag);
    if (tag_concreto == NULL){
        // errno()
        //gestionar mejor el error
        pthread_mutex_unlock(&mutex_files);
        return;
    }

    gestionarTruncateSegunTamanio(tag_concreto,tamanio_a_truncar);
    actualizarTamanioMetadata(nombre_file, tag_concreto,tamanio_a_truncar);

    pthread_mutex_unlock(&mutex_files);
    free(nombre_file);
    free(nombre_tag);
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

File* crearFile(char* nombre_file,char* path_file){
    crearDirectorio(path_file);
    File* nuevo_file = malloc(sizeof(File));
    nuevo_file->nombre_file= nombre_file;
    nuevo_file->tags = list_create();
}

Tag* crearTag(char* nombre_tag, char* path_tag){
    crearDirectorio(nombre_tag);
    Tag* tag = malloc(sizeof(Tag));

    tag->nombre_tag = nombre_tag;
    tag->metadata_config_tag = crearMetadata(path_tag);
    tag->bloques_logicos = list_create();
    tag->tamanio = 0;

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
    
    if(tag_concreto->tamanio == tamanio_a_truncar){
        return true;
    } else if (tag_concreto->tamanio > tamanio_a_truncar){
        ferConLaMexicana(tag_concreto, tamanio_a_truncar); // siempre te podes achicar, sino miralo a Fer
        return true;
    } else {
        return agrandarEnTruncate(tag_concreto,tamanio_a_truncar);
    }
}

//war
void ferConLaMexicana(Tag* tag, int nuevo_tamanio){ // tag tamanio 
    int cant_bloques_a_desasignar = (tag->tamanio - nuevo_tamanio)/ datos_superblock_gb->tamanio_bloque;
    //aca falta algo cuales?
    for(int i = 0; i < cant_bloques_a_desasignar; i++){
        BloqueLogico* bloque_popeado = (BloqueLogico* )list_remove(tag->bloques_logicos,list_size(tag->bloques_logicos));
        BloqueFisico* bloque_fisico_asociado = bloque_popeado->ptr_bloque_fisico;
        
        //TODO
        //unlink al bloque popeado
        //consultar con el path del ... st_nlink para liberar el bitmap
        
        if (){
            liberarBloqueDeBitmap(bloque_fisico_asociado->id_fisico);
        }
        
        liberarBloqueLogico(bloque_popeado); // 
        //TODO hay que ver como le pasamos el id_fisico
    }
    
}




bool agrandarEnTruncate(Tag* tag,int nuevo_tamanio){
    int tamanio_a_agrandar = nuevo_tamanio - tag->tamanio;
    int cant_bloques_necesarios = tamanio_a_agrandar / datos_superblock_gb->tamanio_bloque;

    asignarBloquesFisicosATag(tag,cant_bloques_necesarios);
    // actualizar metadata
    
    tag->tamanio = nuevo_tamanio;

    return true;
}

//Asumo que pase todos los chequeos
void asignarBloquesFisicosATag(Tag* tag_a_asignar_hardlinks,int cant_bloques_necesarios){  // 2 3 4 6 7   
    BloqueFisico* block0 = (BloqueFisico *)list_get(bloques_fisicos_gb,0); //esto puede ser global (block0)

    t_list* logicos_a_asignar = tag_a_asignar_hardlinks->bloques_logicos;
    int cant_bloques_antes_de_asignacion = list_size(logicos_a_asignar); // 5 8 3
    /*
    TRUNCATE 2132
    NECESITO CREAR 3 BLOGICOS (004, 005 Y 006)
    tagv1_2_3
    ---/000
       /001
       /002
       /003
       
    */
    
    //agregar a la metadata cositas
    for (int i =  0; i < cant_bloques_necesarios; i++){
        BloqueLogico* logico_a_crear = crearBloqueLogico(cant_bloques_antes_de_asignacion);
        // se libero  bloque_popeado
        list_add(logicos_a_asignar,logico_a_crear);
        cant_bloques_antes_de_asignacion++;
    }
}

BloqueLogico* crearBloqueLogico(int nro_bloque_logico){
    BloqueLogico* bloque_logico = malloc (sizeof(BloqueLogico));
    BloqueFisico* block0 = (BloqueFisico *)list_get(bloques_fisicos_gb,0);
    bloque_logico->ptr_bloque_fisico = block0;
    bloque_logico->id_logico = nro_bloque_logico;
    crearArchBloqueLogico(nro_bloque_logico, bloque_logico->directorio); 
    return bloque_logico;
}


void crearArchBloqueLogico(int nro_bloque,char* path_directorio_logico){
    char* full_path_logico;
    char* full_path_fisico_block0;
    //----------------- fisico ------------------
    //----------------- block0 ------------------
    sprintf(full_path_fisico_block0,"%sblock0000.dat",PATH_PHYSICAL_BLOCKS);
    
    // ---------------- logico -----------------
    char* nro_bloque_logico_con_ceros = obtenerNombreBloqueConCeros(nro_bloque);
    //---------------------- "/storage/files/arch1/tag1.0/logical_blocks/" + "0010" + ".dat"
    sprintf(full_path_logico,"%s%s.dat",path_directorio_logico, nro_bloque_logico_con_ceros);

    
    FILE* dat_a_crear = fopen(full_path_logico,"a+"); //logical block a crear
    fclose(dat_a_crear);
    
    if(!link(full_path_fisico_block0,full_path_logico)){
        log_error(logger_storage,"ERROR - (crearArchBloqueLogico) - Error al hacer link :(");
        abort();
    }

    log_debug(
        logger_storage,"Debug - (crearArchBloqueLogico) - Bloque %d del directorio %s correctamente creado",nro_bloque,path_directorio_logico);
    
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

//Limpia el bit, hasta el tuje te limpia
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
    free(bloque_a_liberar->directorio);
    free(bloque_a_liberar->nombre);
    free(bloque_a_liberar); 
}
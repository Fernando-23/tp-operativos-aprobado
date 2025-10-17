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
        "Debug - (gestionarClienteIndividual) - Recibi el mensaje %s",mensajito->mensaje);

    // COD_OP QUERY_ID ...
    char** mensajito_cortado = string_split(mensajito->mensaje," ");
    CodOperacionStorage tipo_operacion = obtenerModuloCodOperacion(mensajito_cortado[0]);
    char* query_id = mensajito_cortado[1];
        
    switch (tipo_operacion){
        
        //CREATE QUERY_ID NOMBRE_FILE TAG
    case CREATE:
        char* nombre_file = mensajito_cortado[2];
        char* tag = mensajito_cortado[3];
        
        bool ta_todo_bien = realizarCREATE(query_id,nombre_file,tag);
        if (!ta_todo_bien){
            enviarMensajito(mensajitoError(),mail_laburante,logger_storage);
            string_array_destroy(mensajito_cortado);
            log_error(logger_storage,"{[^&^]}"); // JOE PINO 
            return;
        }

        string_array_destroy(mensajito_cortado);
        
        break;
    case TRUNCATE:
        enviarMensajito(mensajitoOk(), mail_laburante, logger_storage);
        
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



#include "operaciones.h"

void* recursosHumanos(void *args_sin_formato)
{
    int socket_server = *(int *)args_sin_formato;

    while (1)
    {
        int fd_cliente;
        log_debug(logger_storage, "esperando cliente");
        fd_cliente = esperarCliente(socket_server, logger_storage);
        log_debug(logger_storage, "se conecto un cliente");
        pthread_t thread_labubu;
        pthread_create(&thread_labubu, NULL, atenderLaburanteDisconforme, (void *)&fd_cliente);
        //pthread_detach(thread_labubu);
    } // n veces porque hay n workers
}

void *atenderLaburanteDisconforme(void *args_sin_formato)
{
    int fd_cliente = *((int *)args_sin_formato);

    handshake(fd_cliente);

    //while(1){
     //   Mensaje * mensajito = re
    //}
    do
    {
        pedidoDeLaburante(fd_cliente);
    } while (1);
}

void pedidoDeLaburante(int mail_laburante){
    char* nombre_file;
    char* nombre_tag;
    Mensaje *mensajito = recibirMensajito(mail_laburante, logger_storage);

    log_debug(logger_storage,
              "Debug - (pedidoDeLaburante) - Recibi el mensaje %s", mensajito->mensaje);

    // COD_OP QUERY_ID ...
    char **mensajito_cortado = string_split(mensajito->mensaje, " ");
    int tipo_operacion = obtenerTareaCodOperacion(mensajito_cortado[0]);
    int query_id = atoi(mensajito_cortado[1]);

    hacerRetardoOperacion();

    switch (tipo_operacion)
    {

    case CREATE:

        log_debug(logger_storage, "(pedidoDeLaburante) - CREATE");
        // CREATE QUERY_ID FILE TAG
        nombre_file = mensajito_cortado[2];
        nombre_tag = mensajito_cortado[3];

        ErrorStorageEnum resultado_CREATE = realizarCREATE(nombre_file, nombre_tag);

         enviarMensajito(mensajitoResultadoStorage(resultado_CREATE), mail_laburante, logger_storage);
        
        if (resultado_CREATE != OK)
            log_warning(logger_storage, "NO SE PUDO REALIZAR CREATE POR MOTIVO FILE_PREEXISTENTE"); // JOE PINO
         else
            //////////////////////////////////// LOG OBLIGATORIO ////////////////////////////////////////
            log_info(logger_storage, "## %d - File Creado %s:%s",query_id, nombre_file, nombre_tag);
            /////////////////////////////////////////////////////////////////////////////////////////////
        
        string_array_destroy(mensajito_cortado);
        break;

    case TRUNCATE:
        log_debug(logger_storage, "(pedidoDeLaburante) - TRUNCATE");
        // TRUNCATE QUERY_ID FILE TAG TAMANIO
        nombre_file = mensajito_cortado[2];
        nombre_tag = mensajito_cortado[3];
        int tamanio_a_truncar = atoi(mensajito_cortado[4]);

        ErrorStorageEnum resultado_TRUNCATE = realizarTRUNCATE(query_id,nombre_file,nombre_tag, tamanio_a_truncar);

        enviarMensajito(mensajitoResultadoStorage(resultado_TRUNCATE), mail_laburante, logger_storage);

        if (resultado_TRUNCATE != OK)    
            log_warning(logger_storage, "NO SE PUDO REALIZAR TRUNCATE POR MOTIVO %s", NOMBRE_ERRORES[resultado_TRUNCATE]); // JOE PINO
        else 
            //////////////////////////////////// LOG OBLIGATORIO ////////////////////////////////////////
            log_info(logger_storage, "##%d - File Truncado %s:%s - Tamaño: %d",query_id, nombre_file, nombre_tag,tamanio_a_truncar);
            /////////////////////////////////////////////////////////////////////////////////////////////
        string_array_destroy(mensajito_cortado);
        break;

    case TAG:
        log_debug(logger_storage, "(pedidoDeLaburante) - TAG");
        // TAG QUERY_ID FILE_O TAG_O FILE_D TAG_D
        char* nombre_file_origen = mensajito_cortado[2];
        char* nombre_tag_origen = mensajito_cortado[3];
        char* nombre_file_destino = mensajito_cortado[4];
        char* nombre_tag_destino = mensajito_cortado[5];

        ErrorStorageEnum resultado_TAG = realizarTAG(nombre_file_origen,nombre_tag_origen,
             nombre_file_destino, nombre_tag_destino, query_id);
        
        enviarMensajito(mensajitoResultadoStorage(resultado_TAG), mail_laburante, logger_storage);
        if (resultado_TAG != OK)
            log_warning(logger_storage, "NO SE PUDO REALIZAR TAG POR MOTIVO %s", NOMBRE_ERRORES[resultado_TAG]);
        else 
            //////////////////////////////////// LOG OBLIGATORIO ////////////////////////////////////////
            log_info(logger_storage, "## %d - Tag creado %s:%s",query_id, nombre_file_destino, nombre_tag_destino);
            /////////////////////////////////////////////////////////////////////////////////////////////

        string_array_destroy(mensajito_cortado);
        break;

    case COMMIT:
        log_debug(logger_storage, "(pedidoDeLaburante) - COMMIT");
        nombre_file = mensajito_cortado[2];
        nombre_tag = mensajito_cortado[3];
        

        ErrorStorageEnum resultado_COMMIT = realizarCOMMIT(query_id, nombre_file,nombre_tag);

        enviarMensajito(mensajitoResultadoStorage(resultado_COMMIT), mail_laburante, logger_storage);

        if (resultado_COMMIT != OK)
            log_warning(logger_storage, "NO SE PUDO REALIZAR COMMIT POR MOTIVO %s", NOMBRE_ERRORES[resultado_COMMIT]);
        else 
            //////////////////////////////////// LOG OBLIGATORIO ////////////////////////////////////////
            log_info(logger_storage, "## %d - Commit de File:Tag %s:%s", query_id, nombre_file, nombre_tag);
            /////////////////////////////////////////////////////////////////////////////////////////////
        
        string_array_destroy(mensajito_cortado);

        break;

    case DELETE:
        log_debug(logger_storage, "(pedidoDeLaburante) - DELETE");
    // DELETE QUERY_ID FILE:TAG
        nombre_file = mensajito_cortado[2];
        nombre_tag = mensajito_cortado[3];  
        ErrorStorageEnum resultado_DELETE = realizarELIMINAR_UN_TAG(query_id,nombre_file,nombre_tag);

        enviarMensajito(mensajitoResultadoStorage(resultado_DELETE), mail_laburante, logger_storage);
        
        if (resultado_DELETE != OK)
            log_warning(logger_storage, "No se pudo realizar DELETE por motivo %s", NOMBRE_ERRORES[resultado_DELETE]);
        
        else 
            //////////////////////////////////// LOG OBLIGATORIO ////////////////////////////////////////
            log_info(logger_storage,"## %d - Tag Eliminado %s:%s",query_id,nombre_file,nombre_tag);
            /////////////////////////////////////////////////////////////////////////////////////////////

        string_array_destroy(mensajito_cortado);
        break;

    case LEER_BLOQUE: //  QUERY_ID FILE TAG NRO_PAG
    log_debug(logger_storage, "(pedidoDeLaburante) - LEER_BLOQUE");
        nombre_file = mensajito_cortado[2];
        nombre_tag = mensajito_cortado[3];
        int id_bloque_logico = atoi(mensajito_cortado[4]);
        char* contenido_a_leer = NULL;

        ErrorStorageEnum respuesta_LECTURA_BLOQUE = realizarREAD(nombre_file,nombre_tag,id_bloque_logico,&contenido_a_leer); 
        
        char* enviar_formateado = string_from_format("%s %s",NOMBRE_ERRORES[respuesta_LECTURA_BLOQUE], contenido_a_leer);
        Mensaje* mensajito_leido = crearMensajito(enviar_formateado);
        
        enviarMensajito(mensajito_leido, mail_laburante, logger_storage);

        free(enviar_formateado);

        if (respuesta_LECTURA_BLOQUE != OK)
            log_warning(logger_storage, "No se pudo realizar LEER_BLOQUE por motivo %s", NOMBRE_ERRORES[respuesta_LECTURA_BLOQUE]);
        
        else
            //////////////////////////////////// LOG OBLIGATORIO //////////////////////////////////////////
            log_info(logger_storage,"## %d - Bloque Lógico Leído %s:%s - Número de Bloque: %d ",query_id, nombre_file , nombre_tag , id_bloque_logico);
            /////////////////////////////////////////////////////////////////////////////////////////////
        string_array_destroy(mensajito_cortado);
        break;

    case ESCRIBIR_BLOQUE:
        log_debug(logger_storage, "(pedidoDeLaburante) - ESCRIBIR_BLOQUE");
        //ESCRIBIR_BLOQUE QUERY_ID FILE TAG NRO_PAG CONTENIDO
        nombre_file = mensajito_cortado[2];
        nombre_tag = mensajito_cortado[3];
        int id_bloque_logico_a = atoi(mensajito_cortado[4]);
        char* contenido_a_escribir = mensajito_cortado[5];
        
        
        ErrorStorageEnum respuesta_ESCRITURA_BLOQUE = realizarWRITE(query_id, nombre_file, nombre_tag, id_bloque_logico_a, contenido_a_escribir);
        
        enviarMensajito(mensajitoResultadoStorage(respuesta_ESCRITURA_BLOQUE), mail_laburante, logger_storage);
        
        if (respuesta_ESCRITURA_BLOQUE != OK)
            log_warning(logger_storage, "No se pudo realizar ESCRITURA_BLOQUE por motivo %s", NOMBRE_ERRORES[respuesta_ESCRITURA_BLOQUE]);
        
        else
            //////////////////////////////////// LOG OBLIGATORIO ////////////////////////////////////////
            log_info(logger_storage,"## %d - Bloque Lógico Escrito %s:%s - Número de Bloque: %d",query_id,nombre_file,nombre_tag, id_bloque_logico_a);
            /////////////////////////////////////////////////////////////////////////////////////////////
        string_array_destroy(mensajito_cortado);
        break;

    
    case CARTA_DOCUMENTO: // CARTA_DOCUMENTO queryid WORKER_ID
        log_debug(logger_storage, "(pedidoDeLaburante) - CARTA_DOCUMENTO");
        int worker_id = atoi(mensajito_cortado[2]);
        pthread_mutex_lock(&mutex_cant_workers);
        cant_workers_conectados--;
        log_info(logger_storage, "## Se desconecta el Worker %d - Cantidad de Workers: %d", worker_id, cant_workers_conectados);
        pthread_mutex_unlock(&mutex_cant_workers);
        
        break;
    default:
        log_error(logger_storage, "Error - (pedidoDeLaburante) - Codigo de operacion erroneo"); // capaz un abort, quien sabe
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

ErrorStorageEnum realizarCREATE(char *nombre_file, char *nombre_tag)
{

    log_debug(logger_storage, "(realizarCREATE) - Comienzo create");

    if (filePreexistente(nombre_file))
        return FILE_PREEXISTENTE; // chequeo de error del enunciado
    //
    log_debug(logger_storage, "(realizarCREATE) - Pase validaciones de creacion");
    File *nuevo_file = crearFile(nombre_file);
    
    
    list_add(lista_files_gb, nuevo_file);
    Tag *nuevo_tag = crearTag(nombre_tag, nombre_file);

    list_add(nuevo_file->tags, nuevo_tag);
    log_debug(logger_storage,"(realizarCREATE) - Termine correctamente create");
    return OK;
}

ErrorStorageEnum realizarTRUNCATE(int query_id,char* nombre_file, char* nombre_tag, int tamanio_a_truncar)
{

    // chequeo errores
    pthread_mutex_lock(&mutex_files);

    log_debug(logger_storage,"(realizarTRUNCATE) - Cant Files:%d" ,lista_files_gb->elements_count);
    File* fileo = list_get(lista_files_gb,0);
    log_debug(logger_storage,"(realizarTRUNCATE) - Nombre File Estructura:%s, Nombre File Parametro:%s",fileo->nombre_file, nombre_file);
    if (fileInexistente(nombre_file))
    {
        pthread_mutex_unlock(&mutex_files);
        return FILE_INEXISTENTE;
    }

    //log_debug(logger_storage,"existe file");

    File *file = buscarFilePorNombre(nombre_file);

    
    log_debug(logger_storage,"(realizarTRUNCATE) - Cant Tags:%d" ,file->tags->elements_count);
    Tag* tageo = list_get(file->tags,0);
    log_debug(logger_storage,"(realizarTRUNCATE) - Nombre Tag Estructura:%s, Nombre Tag Parametro:%s",tageo->nombre_tag, nombre_tag);

    if (tagInexistente(file->tags, nombre_tag, nombre_file))
    {
        log_debug(logger_storage,"(realizarTRUNCATE) - TAG INEXISTENTE, O SEA NO EXISTe");
        pthread_mutex_unlock(&mutex_files);
        return TAG_INEXISTENTE;
    }



    Tag *tag_concreto = buscarTagPorNombre(file->tags, nombre_tag);

    if (!tag_concreto) {
        log_error(logger_storage, "(realizarTRUNCATE) - ERROR: tag_concreto es NULL");
        pthread_mutex_unlock(&mutex_files);
        return TAG_INEXISTENTE;
    }

    gestionarTruncateSegunTamanio(tag_concreto,query_id, tamanio_a_truncar,nombre_file,nombre_tag);
    actualizarTamanioMetadata(nombre_file, tag_concreto, tamanio_a_truncar);

    pthread_mutex_unlock(&mutex_files);

    return OK;
}

void hacerRetardoOperacion(){
    usleep(config_storage->retardo_operacion  * 1000); 
}

void actualizarTamanioMetadata(char *nombre_file, Tag *tag, int tamanio_a_truncar)
{
    t_config *metadata_a_actualizar = tag->metadata_config_tag;
    config_set_value(metadata_a_actualizar, "TAMANIO", string_itoa(tamanio_a_truncar));
    config_save(metadata_a_actualizar);
    log_debug(logger_storage, "Debug - (actualizarTamanioMetadata) - Actualizo metadata-File:%s - Tag: %s - Tamanio: %d",
              nombre_file, tag->nombre_tag, tamanio_a_truncar);
}

void crearDirectorio(char *path_directorio)
{
    struct stat st = {0};
    if (stat(path_directorio, &st) == -1)
    {
        mkdir(path_directorio, 0777);
    }
}

File *crearFile(char *nombre_file)
{
    log_debug(logger_storage, "(crearFile) - Intentando crear file %s", nombre_file);
    File *nuevo_file = malloc(sizeof(File));
    nuevo_file->nombre_file = string_duplicate(nombre_file); // LIBERAR
    char *dir_a_crear = string_from_format("%s/%s", RUTA_FILES, nuevo_file->nombre_file);
    crearDirectorio(dir_a_crear); // mkdir como Agustin Coda

    nuevo_file->tags = list_create();

     log_debug(logger_storage, "(crearFile) - File %s creado", nombre_file);

    return nuevo_file;
}

Tag *crearTag(char *nombre_tag, char *nombre_file_asociado)
{
    log_debug(logger_storage, "(crearTag) - Intentando crear tag %s", nombre_tag);
    Tag *tag = malloc(sizeof(Tag));

    tag->nombre_tag = string_duplicate(nombre_tag);

    // home/utnso/rerewr/reewrwe/ewr/wer/nombrefile/nombretag
    tag->directorio = string_from_format("%s/%s/%s", RUTA_FILES, nombre_file_asociado, tag->nombre_tag);
    log_debug(logger_storage, "(crearTag) - Directorio a crear: %s", tag->directorio);
    crearDirectorio(tag->directorio);

    // Creo carpeta logical blocks
    log_debug(logger_storage, "(crearTag) - Creando directorio logical_blocks en: %s", tag->directorio);
    char *ruta_aux_lblocks = string_from_format("%s/logical_blocks", tag->directorio);
    crearDirectorio(ruta_aux_lblocks);
    free(ruta_aux_lblocks);
    
    log_debug(logger_storage, "(crearTag) - Directorio logical_blocks creado en: %s", tag->directorio);
    tag->metadata_config_tag = crearMetadata(tag->directorio);
    log_debug(logger_storage, "(crearTag) - Metadata creada en: %s/metadata.config", tag->directorio);
    
    tag->bloques_logicos = list_create();

    log_debug(logger_storage, "Debug - (crearTag) - Se creo el tag %s", nombre_tag);

    return tag;
}

t_config *crearMetadata(char *path_tag)
{
    char *path_metadata = string_from_format("%s/metadata.config", path_tag);
    log_debug(logger_storage, "(crearMetadata) - path metadata: %s", path_metadata);
    FILE *arch_config = fopen(path_metadata, "a+");
    fclose(arch_config);
    t_config *metadata = config_create(path_metadata);
    config_set_value(metadata, "TAMANIO", "0");
    config_set_value(metadata, "BLOCKS", "[]");
    config_set_value(metadata, "ESTADO", "WORK_IN_PROGRESS");

    config_save_in_file(metadata, path_metadata);
   
    free(path_metadata);
    return metadata;
}

// tenes que hacer free nombre_file y tag
void asignarFileTagAChars(char *nombre_file, char *tag, char *file_a_cortar)
{
    char **file_cortado = string_split(file_a_cortar, ":"); // FILE:TAG

    nombre_file = string_duplicate(file_cortado[0]); // LIBERAR
    tag = string_duplicate(file_cortado[1]);         // LIBERAR

    string_array_destroy(file_cortado);
}

File *buscarFilePorNombre(char *nombre)
{

    bool tieneMismoNombre(void *ptr)
    {
        File *file = (File *)ptr;
        return (string_equals_ignore_case(nombre, file->nombre_file));
    }
    return list_find(lista_files_gb, tieneMismoNombre);
}

Tag *buscarTagPorNombre(t_list *tags, char *nombre_tag)
{
    log_debug(logger_storage, "DEBUG -(buscarTagPorNombre)");
    bool tieneMismoNombreTag(void *ptr)
    {
        Tag *tag = (Tag *)ptr;
        return (string_equals_ignore_case(nombre_tag, tag->nombre_tag));
    }

    return list_find(tags, tieneMismoNombreTag);
}


void gestionarTruncateSegunTamanio(Tag *tag_concreto,int query_id, int tamanio_a_truncar,char* nombre_file,char* nombre_tag)
{
    log_debug(logger_storage, "DEBUG -(gestionarTruncateSegunTamanio)");

    int tamanio_actual = config_get_int_value(tag_concreto->metadata_config_tag, "TAMANIO");

    if (tamanio_actual == tamanio_a_truncar)
    {
        log_debug(logger_storage, "DEBUG -(gestionarTruncateSegunTamanio)- Mismo nuevo tamanio recibido en TRUNCATE");
    }
    else if (tamanio_actual > tamanio_a_truncar){
        ferConLaMexicana(tag_concreto,query_id, tamanio_actual, tamanio_a_truncar,nombre_file,nombre_tag); // siempre te podes achicar, sino miralo a Fer
    }
    else{ // tamanio_actual < tamanio_a_truncar
        agrandarEnTruncate(tag_concreto, tamanio_actual, tamanio_a_truncar);
    }    
    
}

void ferConLaMexicana(Tag *tag, int query_id,int tamanio_actual, int nuevo_tamanio,char* nombre_file,char* nombre_tag){ // tag tamanio
    log_debug(logger_storage, "DEBUG -(ferConLaMexicana)- f fer");
    int cant_bloques_a_desasignar = (tamanio_actual - nuevo_tamanio) / datos_superblock_gb->tamanio_bloque;
    unlinkearBloquesLogicos(query_id,cant_bloques_a_desasignar, tag->bloques_logicos,nombre_file,nombre_tag);
}

void unlinkearBloquesLogicos(int query_id,int cant_a_unlinkear, t_list *bloques_logicos,char* nombre_file,char* nombre_tag)
{
    for (int i = 0; i < cant_a_unlinkear; i++){
        BloqueLogico *bloque_popeado = (BloqueLogico *)list_remove(bloques_logicos, list_size(bloques_logicos) - 1 );
        BloqueFisico *bloque_fisico_asociado = bloque_popeado->ptr_bloque_fisico;

        //---------------------------------- /files/tag/logical-blocks/0002.dat
        unlink(bloque_popeado->ruta_hl);       // quitarle el hlink
        log_info(logger_storage,"## %d - %s:%s Se eliminó el hard link del bloque lógico %d al bloque físico %d",
        query_id,nombre_file,nombre_tag,bloque_popeado->id_logico,bloque_popeado->ptr_bloque_fisico->id_fisico);
        bloque_popeado->ptr_bloque_fisico = NULL; // JORGE EL CURIOSO - Capaz por enunciado deberia apuntar al block0;

        if (!tieneHLinks(bloque_fisico_asociado->ruta_absoluta)){ // 1 hard links -> liberar en el bitmap
            liberarBloqueDeBitmap(bloque_fisico_asociado->id_fisico, query_id);
            log_debug(logger_storage, "(unlinkearBloquesLogicos)- El bloque fisico %s fue liberado", bloque_fisico_asociado->nombre);
        }

        liberarBloqueLogico(bloque_popeado);
        log_debug(logger_storage, "(unlinkearBloquesLogicos) - FER lo hizo de nuevo");
    }
}

void agrandarEnTruncate(Tag *tag, int tamanio_acutal, int nuevo_tamanio)
{
    log_debug(logger_storage, "DEBUG -(agrandarEnTruncate)- vamos fer");
    int tamanio_a_agrandar = nuevo_tamanio - tamanio_acutal;
    int cant_bloques_necesarios = tamanio_a_agrandar / datos_superblock_gb->tamanio_bloque;

    asignarBloquesFisicosATagEnTruncate(tag, cant_bloques_necesarios);
    // actualizar metadata
    config_set_value(tag->metadata_config_tag, "TAMANIO", string_itoa(nuevo_tamanio));
    config_save(tag->metadata_config_tag);
}

void asignarBloquesFisicosATagEnTruncate(Tag *tag_a_asignar_hardlinks, int cant_bloques_necesarios)
{
    BloqueFisico *block0 = (BloqueFisico *)list_get(bloques_fisicos_gb, 0); // esto puede ser global (block0)
    
    t_list *logicos_a_asignar = tag_a_asignar_hardlinks->bloques_logicos;

    int cant_bloques_antes_de_asignacion = list_size(logicos_a_asignar); // 5 8 3
    

    char **bloques_logicos = config_get_array_value(tag_a_asignar_hardlinks->metadata_config_tag, "BLOCKS");
    
    for (int i = 0; i < cant_bloques_necesarios; i++)
    {
        BloqueLogico *logico_a_crear = crearBloqueLogico(cant_bloques_antes_de_asignacion, block0, tag_a_asignar_hardlinks->directorio);

        string_array_push(&bloques_logicos, string_duplicate("0")); // LIBERAR

        list_add(logicos_a_asignar, logico_a_crear);
        cant_bloques_antes_de_asignacion++;
    }

    char *nueva_info_blocks_metadata = stringArrayConfigAString(bloques_logicos);
    log_debug(logger_storage,"carga los bloques logicos en la config (asignarBloquesFisicosATagEnTruncate-");
    

    config_set_value(tag_a_asignar_hardlinks->metadata_config_tag, "BLOCKS", nueva_info_blocks_metadata);
    config_save(tag_a_asignar_hardlinks->metadata_config_tag);

    free(nueva_info_blocks_metadata);
    string_array_destroy(bloques_logicos);
}

// ----------------------/home/utnso/storage/files/tag1_0_0
BloqueLogico *crearBloqueLogico(int nro_bloque_logico, BloqueFisico *bloque_fisico_a_asignar, char *path_tag)
{ //-- ta checkkkk  -- update 22/11/25 que hijo de puta, firma Joe
    BloqueLogico *bloque_logico = malloc(sizeof(BloqueLogico));
    bloque_logico->ptr_bloque_fisico = bloque_fisico_a_asignar;
    bloque_logico->id_logico = nro_bloque_logico;
    bloque_logico->ruta_hl = string_from_format("%s/logical_blocks/%04d.dat", path_tag, nro_bloque_logico);    

    
    

    // Crear el directorio logical_blocks si no existe
    char *dir_logical_blocks = string_from_format("%s/logical_blocks", path_tag);
    crearDirectorio(dir_logical_blocks);
    log_debug(logger_storage, "(crearArchBloqueLogico) - CREE directorio )");
    free(dir_logical_blocks);


    if (!crearHLink(bloque_logico->ruta_hl, bloque_fisico_a_asignar->ruta_absoluta))
    {
        log_error(logger_storage, "(crearArchBloqueLogico) - Error al hacer link :(");
        abort();
    }

    log_debug(
        logger_storage, "(crearArchBloqueLogico) - Bloque %d del directorio %s correctamente creado", nro_bloque_logico, bloque_logico->ruta_hl);
    return bloque_logico;
}

// ESTA CHECKKKK
bool crearHLink(char *ruta_hl_del_bloque_logico, char *bloque_fisico_a_hardlinkear)
{
    if (link(bloque_fisico_a_hardlinkear, ruta_hl_del_bloque_logico) == -1){
        log_error(logger_storage, "(crearHLink) - fallo link");
        return false;
    }
        
    //log_info(logger_storage,"## %d - :<TAG> Se agregó el hard link del bloque lógico <BLOQUE_LOGICO> al bloque físico <BLOQUE_FISICO>",,);
    return true;
}


// bool crearHLink(char *ruta_hl_del_bloque_logico, char *bloque_fisico_a_hardlinkear)
// {
//     // A esta altura el directorio de ruta_hl_del_bloque_logico debe existir.
//     // No hay que crear el archivo destino.

//     if (link(bloque_fisico_a_hardlinkear, ruta_hl_del_bloque_logico) == -1) {
//         log_error(
//             logger_storage,
//             "(crearHLink) - fallo link. old='%s' new='%s' errno=%d (%s)",
//             bloque_fisico_a_hardlinkear,
//             ruta_hl_del_bloque_logico,
//             errno,
//             strerror(errno)
//         );
//         return false;
//     }

//     log_debug(
//         logger_storage,
//         "(crearHLink) - hard link creado ok. old='%s' new='%s'",
//         bloque_fisico_a_hardlinkear,
//         ruta_hl_del_bloque_logico
//     );
//     return true;
// }

void eliminarRespuestaConsultaBitmap(RespuestaConsultaBitmap *response_a_limpiar)
{
    string_array_destroy(response_a_limpiar->bloques_encontrados);
    free(response_a_limpiar);
}

void limpiarBitsPorStringArray(char **bloques_a_limpiar)
{
    int tope_array = string_array_size(bloques_a_limpiar);

    for (int i = 0; i < tope_array; i++)
    {
        char *bloque_popeado = string_array_pop(bloques_a_limpiar);
        int nro_bloque = atoi(bloque_popeado);
        free(bloque_popeado);
        bitarray_clean_bit(bitmap_gb, nro_bloque);
    }
    log_debug(logger_storage, "Debug - (limpiarBitsPorStringArray) - Se liberaron bloques del bitmap");
}

void liberarBloqueDeBitmap(int nro_bloque, int query_id){
    bitarray_clean_bit(bitmap_gb, nro_bloque);
    log_info(logger_storage, "## %d - Bloque Físico Liberado - Número de Bloque: %d",query_id, nro_bloque);
}



void liberarBloqueLogico(BloqueLogico *bloque_a_liberar)
{
    log_debug(logger_storage, "(liberarBloqueLogico) - voy a liberar bloque el bloque logico");
    log_debug(logger_storage, "(liberarBloqueLogico) - id_logido: %d", bloque_a_liberar->id_logico);
    log_debug(logger_storage, "(liberarBloqueLogico) - nombre: ",bloque_a_liberar->nombre);
    log_debug(logger_storage, "(liberarBloqueLogico) - ruta_hl: %s",bloque_a_liberar->ruta_hl);


    
    free(bloque_a_liberar->ruta_hl);
    log_debug(logger_storage,"(liberarBloqueLogico) - libere ruta_hl");
    free(bloque_a_liberar->nombre);
    log_debug(logger_storage,"(liberarBloqueLogico) - libere nombre");
    free(bloque_a_liberar);
    log_debug(logger_storage,"(liberarBloqueLogico) - libere bloque");
    log_debug(logger_storage, "(liberarBloqueLogico) - bloque logico liberado");
}

int contadorHLinks(char *ruta_abs_a_consultar)
{
    struct stat info;
    if (stat(ruta_abs_a_consultar, &info) == -1)
    {
        log_error(logger_storage, "(contadorHLinks) -Error en stat, Ruta: %s", ruta_abs_a_consultar);
         return 1;
    }

    return info.st_nlink;
}


bool tieneHLinks(char *ruta_abs_a_consultar)
{
    struct stat info;
    if (stat(ruta_abs_a_consultar, &info) == -1)
    {
        log_error(logger_storage, "(tieneHLinks) - Error en stat, Ruta: %s", ruta_abs_a_consultar);
        abort();
    }

    return (info.st_nlink > 1);
}


// LIBERAR STRING RETORNADO CON CoFREE con Leche
char *stringArrayConfigAString(char **array_a_pasar_a_string)
{
    char *string_a_retornar = string_new();
    string_append(&string_a_retornar, "[");

    for (int i = 0; i < string_array_size(array_a_pasar_a_string); i++)
    {

        string_append(&string_a_retornar, array_a_pasar_a_string[i]);

        if (i != string_array_size(array_a_pasar_a_string) - 1)
        {
            string_append(&string_a_retornar, ",");
        }
    }
    string_append(&string_a_retornar, "]");
    return string_a_retornar;
}

ErrorStorageEnum realizarTAG(char *nombre_file_origen,
char* nombre_tag_origen, char* nombre_file_destino,char* nombre_tag_destino, int query_id){

    // ======================== chequeo errores
    pthread_mutex_lock(&mutex_files);
    // NO EXISTE FILE ORIGEN
    if (fileInexistente(nombre_file_origen)){

        log_debug(logger_storage,"(realizarTAG) - El file origen %s no existe:",nombre_file_origen);
        pthread_mutex_unlock(&mutex_files);
        return FILE_INEXISTENTE;
    }

    File *file_origen = buscarFilePorNombre(nombre_file_origen);
    // NO EXISTE TAG ORIGEN
    if (tagInexistente(file_origen->tags, nombre_tag_origen, nombre_file_origen))
    {
        log_debug(logger_storage,"(realizarTAG) - No existe el tag origen %s en el file origen %s",nombre_tag_origen,nombre_file_origen);
        pthread_mutex_unlock(&mutex_files);
        return TAG_INEXISTENTE;
    }
    

    // CASOS
    // file creado
    // file tag creado
    // ninguno creado

    // NINGUNO CREADO
    File *file_destino_estructura;
    Tag *tag_destino_estructura;

    if (!filePreexistente(nombre_file_destino))
    {
        log_debug(logger_storage,"(realizarTAG) - No existe el file, intentando crear file destino %s y tag destino %s",nombre_file_destino,nombre_tag_destino);
        file_destino_estructura = crearFile(nombre_file_destino);
        tag_destino_estructura = crearTag(nombre_tag_destino, nombre_file_destino);
        list_add(lista_files_gb,file_destino_estructura);
        list_add(file_destino_estructura->tags, tag_destino_estructura);
        asignarBloquesFisicosATagCopiado(tag_destino_estructura,nombre_file_destino, query_id);

        pthread_mutex_unlock(&mutex_files);
        
        return OK;
    }

    // FILE CREADO, TAG INEXISTENTE
    file_destino_estructura = buscarFilePorNombre(nombre_file_destino);
    if (!tagPreexistente(file_destino_estructura->tags, nombre_tag_destino, nombre_file_destino))
    {
        log_debug(logger_storage,"(realizarTAG) - El tag destino %s no existe, voy a intentar crearlo jejeje",nombre_file_origen);
        
        // BUSCAR EL TAG ORIGEN PARA COPIAR SU METADATA
        Tag *tag_origen = buscarTagPorNombre(file_origen->tags, nombre_tag_origen);
        
        // CREAR EL TAG DESTINO
        tag_destino_estructura = crearTag(nombre_tag_destino, nombre_file_destino);
        
        // COPIAR LA METADATA DEL TAG ORIGEN AL TAG DESTINO
        int tamanio_origen = config_get_int_value(tag_origen->metadata_config_tag, "TAMANIO");
        char **bloques_origen = config_get_array_value(tag_origen->metadata_config_tag, "BLOCKS");
        char *bloques_string = stringArrayConfigAString(bloques_origen);
        
        char* tamanio_string = string_itoa(tamanio_origen);
        config_set_value(tag_destino_estructura->metadata_config_tag, "TAMANIO", tamanio_string);
        config_set_value(tag_destino_estructura->metadata_config_tag, "BLOCKS", bloques_string);
        config_set_value(tag_destino_estructura->metadata_config_tag,"ESTADO","WORK_IN_PROGRESS");
        config_save(tag_destino_estructura->metadata_config_tag);
        free(tamanio_string);
        log_debug(logger_storage, "(realizarTAG) - tamanio tag copiado: %d", tamanio_origen);
        
        // AHORA SÍ, ASIGNAR LOS BLOQUES FÍSICOS AL TAG DESTINO
        asignarBloquesFisicosATagCopiado(tag_destino_estructura, nombre_file_destino, query_id);
        list_add(file_destino_estructura->tags, tag_destino_estructura);

        free(bloques_string);
        string_array_destroy(bloques_origen);

        pthread_mutex_unlock(&mutex_files);
        
        return OK;
    }

    // TODO CREADO, SOLO COPIA
    log_debug(logger_storage,"(realizarTAG) - File y Tag destino ya existen, solo se copia la info del tag origen %s al tag destino %s",nombre_tag_origen,nombre_tag_destino);
    tag_destino_estructura = buscarTagPorNombre(file_destino_estructura->tags, nombre_tag_destino);
    
    if (!tag_destino_estructura) {
        log_error(logger_storage, "(realizarTAG) - ERROR: No se encontro tag destino %s en file %s", nombre_tag_destino, nombre_file_destino);
        pthread_mutex_unlock(&mutex_files);
        return TAG_INEXISTENTE;
    }

    asignarBloquesFisicosATagCopiado(tag_destino_estructura,nombre_file_destino, query_id);
    
    pthread_mutex_unlock(&mutex_files);
    return OK;
}

/*int copiarTag(Tag *tag_origen, Tag *tag_destino, char* nombre_file, int query_id)
{

    char *aux_path_metadata_destino;
    //------------------------------- PROBAR  SI REALMENTE LO COPIA EN EL DESTINO
    sprintf(aux_path_metadata_destino, "%s/metadata.config", tag_destino->directorio);

    config_save_in_file(tag_origen->metadata_config_tag, aux_path_metadata_destino);
    config_set_value(tag_destino->metadata_config_tag, "ESTADO", "WORK_IN_PROGRESS");

    //------------------------------- tag_destino ---------------> [10,2,0,0,0,0]
    asignarBloquesFisicosATagCopiado(tag_destino, nombre_file, query_id);

    
}*/

void asignarBloquesFisicosATagCopiado(Tag *tag_destino, char* nombre_file, int query_id) // falta query id
{
    char **bloques_logicos_a_copiar = config_get_array_value(tag_destino->metadata_config_tag, "BLOCKS");
    t_list *logicos_a_copiar = tag_destino->bloques_logicos;

    if (!list_is_empty(logicos_a_copiar))
    { // caso donde el tag estaba creado y CAPAZ tenia blogicos asignados
        log_debug(logger_storage, "(asignarBloquesFisicosATagCopiado) - El tag %s destino tenia cosas, se liberaron",tag_destino->nombre_tag);
        unlinkearBloquesLogicos(query_id,list_size(logicos_a_copiar), logicos_a_copiar,nombre_file,tag_destino->nombre_tag);
    }

    for (int i = 0; i < string_array_size(bloques_logicos_a_copiar); i++)
    {
        int nro_bloque_fisico = atoi(bloques_logicos_a_copiar[i]);
        BloqueFisico *bloque_fisico_a_asignar = obtenerBloqueFisico(nro_bloque_fisico);
        BloqueLogico *logico_a_crear = crearBloqueLogico(i, bloque_fisico_a_asignar, tag_destino->directorio);

        list_add(logicos_a_copiar, logico_a_crear);
    }

    log_debug(logger_storage, "(asignarBloquesFisicosATagCopiado) - Bloques asignados");
    string_array_destroy(bloques_logicos_a_copiar);
}

BloqueFisico *obtenerBloqueFisico(int nro_bloque_a_buscar){
    return (BloqueFisico *)list_get(bloques_fisicos_gb, nro_bloque_a_buscar);
}


ErrorStorageEnum realizarCOMMIT(int query_id, char *nombre_file,char *nombre_tag){

    if (fileInexistente(nombre_file)){
        
        return FILE_INEXISTENTE;
    }
    File *file_a_commitear = buscarFilePorNombre(nombre_file);

    if (tagInexistente(file_a_commitear->tags, nombre_tag, nombre_file)){
        
        return TAG_INEXISTENTE;
    }

    Tag *tag_a_commitear = buscarTagPorNombre(file_a_commitear->tags, nombre_tag);

    if (tieneEstadoCOMMITED(tag_a_commitear))
    {
        log_debug(logger_storage, "(realizarCOMMIT) - El estado del tag %s ya estaba en COMMITED", tag_a_commitear->nombre_tag);
        
        return OK;
    }

    escribirEnHashIndex(query_id,nombre_file,tag_a_commitear);
    
    
    return OK;
}
/*
Bloques Logicos. Cada uno apunta a un bloque fisico.
Commitear: 5: cambie 2
*/


void escribirEnHashIndex(int query_id,char* nombre_file,Tag *tag)
{
    int cant_blogicos = list_size(tag->bloques_logicos);
    
    for (int i = 0; i < cant_blogicos; i++)
    {
        BloqueLogico *bloque_logico_a_consultar = list_get(tag->bloques_logicos, i);
        DatosParaHash *datos_para_hash = obtenerDatosParaHash(bloque_logico_a_consultar);

        char *hash_bloque_logico = crypto_md5(datos_para_hash->contenido, datos_para_hash->tamanio); // block0000 // 9
        bool existe_hash = config_has_property(hash_index_config_gb,hash_bloque_logico);
        
        
        if (existe_hash){
            int id_fisico_actual = bloque_logico_a_consultar->ptr_bloque_fisico->id_fisico;
            log_debug(logger_storage,"(escribirEnHashIndex) - Hash encontrado en el index config, tratando de reapuntar");
            
            char* bloque_fisico_de_config = string_duplicate(config_get_string_value(hash_index_config_gb,hash_bloque_logico));
            
            BloqueFisico* bloque_fisico_a_apuntar = buscarBloqueFisicoPorNombre(bloque_fisico_de_config);

            log_info(logger_storage, "%d - %s:%s Bloque Lógico %d se reasigna de %d a %d"
            ,query_id, nombre_file, tag->nombre_tag, bloque_logico_a_consultar->id_logico , id_fisico_actual, bloque_fisico_a_apuntar->id_fisico);

            
            reapuntarBloque(bloque_fisico_a_apuntar,bloque_logico_a_consultar, tag->metadata_config_tag, query_id, nombre_file, tag->nombre_tag);
            
            
            
            log_info(logger_storage,
            "## %d - %s:%s Se agregó el hard link del bloque lógico %d al bloque físico %d",
            query_id,nombre_file,tag->nombre_tag,bloque_logico_a_consultar->id_logico,bloque_fisico_a_apuntar->id_fisico);

            
            
            log_debug(logger_storage,
                "(escribirEnHashIndex) - Bloque logico %d reapuntado al Bloque fisico %d correctamente",
                bloque_logico_a_consultar->id_logico,bloque_fisico_a_apuntar->id_fisico);
            free(hash_bloque_logico);
            free(bloque_fisico_de_config);
            liberarDatosParaHash(datos_para_hash);
            continue;
        }

        //caso 2, no esta en la config de los hush, como el isaac 
        config_set_value(hash_index_config_gb,
            hash_bloque_logico,
            bloque_logico_a_consultar->ptr_bloque_fisico->nombre); //creo key mi bro no se me preocupe, cierre los ojos y salte nomas.
        config_save(hash_index_config_gb);
        free(hash_bloque_logico);
        liberarDatosParaHash(datos_para_hash);
    }
    
    log_debug(logger_storage, "Yo sali del barrio me dicen MOMO");
}


BloqueFisico *buscarBloqueFisicoPorNombre(char *nombre){
    bool tieneMismoNombre(void *ptr)
    {
        BloqueFisico *bloque = (BloqueFisico *)ptr;
        return (strcmp(nombre, bloque->nombre));
    }
    return list_find(bloques_fisicos_gb, tieneMismoNombre);
}

BloqueFisico* buscarBloqueFisicoPorId(int id_fisico){
    
    bool tieneMismoId(void *ptr)
    {
        BloqueFisico *bloque = (BloqueFisico *)ptr;
        return bloque->id_fisico == id_fisico ;
    }
    return list_find(bloques_fisicos_gb, tieneMismoId);
}

BloqueLogico* buscarBloqueLogicoPorId(Tag* tag, int id_logico) {
    if (!tag || !tag->bloques_logicos) {
        return NULL;
    }

    bool tieneMismoId(void* ptr) {
        BloqueLogico* bloque = (BloqueLogico*)ptr;
        return bloque->id_logico == id_logico;
    }

    return list_find(tag->bloques_logicos, tieneMismoId);
}

DatosParaHash *obtenerDatosParaHash(BloqueLogico *bloque_logico){
    FILE *arch_a_leer = fopen(bloque_logico->ptr_bloque_fisico->ruta_absoluta, "rw");
    if (!arch_a_leer)
        abort();

    fseek(arch_a_leer, 0, SEEK_END);

    long tamanio = ftell(arch_a_leer);
    rewind(arch_a_leer);

    void *contenido = malloc((size_t)tamanio);
    fread(contenido, 1, (size_t)tamanio, arch_a_leer);
    fclose(arch_a_leer);

    return crearDatosHash(contenido, (size_t)tamanio);
}

DatosParaHash *crearDatosHash(void *contenido, size_t tamanio)
{
    DatosParaHash *datos = malloc(sizeof(DatosParaHash));
    datos->tamanio = tamanio;
    datos->contenido = malloc(datos->tamanio);
    datos->contenido = contenido;

    return datos;
}

void liberarDatosParaHash(DatosParaHash *dato_a_liberar){
    free(dato_a_liberar->contenido);
    free(dato_a_liberar);
}

ErrorStorageEnum realizarWRITE(int query_id,char* nombre_file,char* nombre_tag,int id_bloque_logico,char* contenido){ 
    // ESCRIBIR_BLOQUE query_id file tag nro_pag contenido
    
    if (fileInexistente(nombre_file)){
        return FILE_INEXISTENTE;
    }

    File *file_a_escribir = buscarFilePorNombre(nombre_file);

    if (tagInexistente(file_a_escribir->tags, nombre_tag, nombre_file)){
        return TAG_INEXISTENTE;
    }

    Tag *tag_a_escribir = buscarTagPorNombre(file_a_escribir->tags, nombre_tag);


    log_debug(logger_storage,"Encontré el tag a escribir y es %s",tag_a_escribir->nombre_tag);
   

    t_list* bloques_logicos_asociados = tag_a_escribir->bloques_logicos;


    if (bloques_logicos_asociados == NULL){
        log_debug(logger_storage, "(realizarWRITE) - bloques_logicos_asociado a tag == NULL");
    }


    if (list_size(bloques_logicos_asociados) <= id_bloque_logico) {
        return ESCRITURA_FUERA_DE_LIMITE;
    }

    log_debug(logger_storage, "(realizarWRITE) pase los checkeos");

    if (tieneEstadoCOMMITED(tag_a_escribir)){
        log_debug(logger_storage, "(realizarWRITE) - El estado del tag %s ya estaba en COMMITED", tag_a_escribir->nombre_tag);    
        return ESCRITURA_NO_PERMITIDA;
    }

    BloqueLogico* bloque_a_escribir = buscarBloqueLogicoPorId(tag_a_escribir,id_bloque_logico);
    
    // caso feo/fer 
    if (contadorHLinks(bloque_a_escribir->ruta_hl) > 2){  
        log_debug(logger_storage, "caso feo/fer ");
        BloqueFisico* bloque_nuevo_a_asignar = obtenerBloqueFisicoLibre();
        log_debug(logger_storage, "(realizarWRITE) - id bloque fisico: %d", bloque_nuevo_a_asignar->id_fisico);
        if (!bloque_nuevo_a_asignar){
            log_warning(logger_storage,"(realizarWRITE) - No se encontro bloque fisico libre en el bitmap");
            return ESPACIO_INSUFICIENTE;
        }
    
        reapuntarBloque(bloque_nuevo_a_asignar,bloque_a_escribir,tag_a_escribir->metadata_config_tag,query_id,file_a_escribir->nombre_file,tag_a_escribir->nombre_tag);
        log_info(logger_storage, "“## %d - Bloque Físico Reservado - Número de Bloque: %d",query_id,bloque_nuevo_a_asignar->id_fisico);
    
    } // caso lindo/liam
    
    log_debug(logger_storage, "caso lindo/liam ");
    escribirEnBloque(bloque_a_escribir->ruta_hl,contenido);
    log_debug(logger_storage,"Escribio en bloque (realizarWRITE)");
    return OK;
}

ErrorStorageEnum realizarREAD(char* nombre_file,char* nombre_tag,int id_bloque_logico,char** contenido){
    char* contenido_a_cargar = *contenido;
    if (fileInexistente(nombre_file)){
        contenido_a_cargar = string_duplicate("");
        return FILE_INEXISTENTE;
    }

    File *file_a_leer = buscarFilePorNombre(nombre_file);


    if (tagInexistente(file_a_leer->tags, nombre_tag, nombre_file)){
        contenido_a_cargar = string_duplicate("");
        return TAG_INEXISTENTE;
    }

    // ESTAMOS ACA


    Tag *tag_a_leer = buscarTagPorNombre(file_a_leer->tags, nombre_tag);
   
    t_list* bloques_logicos_asociados = tag_a_leer->bloques_logicos;

    if (bloques_logicos_asociados == NULL){
        log_debug(logger_storage, "(realizarREAD) - bloques_logicos_asociado a tag == NULL");
    }
    log_debug(logger_storage," (realizarREAD) - cant_bloques_logicos: %d", bloques_logicos_asociados->elements_count);
    if (list_size(bloques_logicos_asociados) <= id_bloque_logico) {
        contenido_a_cargar = string_duplicate("");
        return LECTURA_FUERA_DE_LIMITE;
    }
    log_debug(logger_storage," (realizarREAD) - Pase validaciones");
    BloqueLogico* bloque_a_leer = buscarBloqueLogicoPorId(tag_a_leer, id_bloque_logico);
    log_debug(logger_storage," (realizarREAD) - Tengo el bloques logico a leer");
    contenido_a_cargar = leerBloqueLogico(bloque_a_leer);
    return OK;
}

BloqueFisico* obtenerBloqueFisicoLibre(){
    if(bitmap_gb == NULL){
        log_error(logger_storage, "ERROR: bitmap_gb es NULL en obtenerBloqueFisicoLibre");
        return NULL;
    }
    
    int max_bits = (int)bitarray_get_max_bit(bitmap_gb);
    //log_debug(logger_storage,"Voy a intentar buscar un bloque fisico libre, bitarray: %d", max_bits);

    for (int i = 0; i < max_bits; i++){
        //log_debug(logger_storage,"entro a for de (obtenerBloqueFisicoLibre)");

        if (!bitarray_test_bit(bitmap_gb,i)){
            bitarray_set_bit(bitmap_gb,i); // lo pones en 1
            log_debug(logger_storage, "(obtenerBloqueFisicoLibre) - Encontre bloque fisico libre en bitmap, id:%d", i);
            return buscarBloqueFisicoPorId(i);
        }
    }
    log_warning(logger_storage,"(obtenerBloqueFisicoLibre) - No se encontro un bloque fisico libre");
    return NULL;
}




void escribirEnBloque(char* ruta_abs_bloque_logico,char* contenido){

    FILE* bloque_a_escribir = fopen(ruta_abs_bloque_logico, "r+b");
    if (!bloque_a_escribir){
        log_error(logger_storage,"(escribirEnBloque) - No se encontro el archivo en la ruta %s", ruta_abs_bloque_logico);
        return;     
    }

    fwrite(contenido, 1, datos_superblock_gb->tamanio_bloque, bloque_a_escribir);\

    if (fflush(bloque_a_escribir) != 0) {
        log_error(logger_storage, "escribirEnBloque: fallo en fflush() para %s", ruta_abs_bloque_logico);
        fclose(bloque_a_escribir);
    }

    if (fsync(fileno(bloque_a_escribir)) != 0) {
        log_error(logger_storage, "escribirEnBloque: fallo crítico en fsync() para %s - %s",
                  ruta_abs_bloque_logico, strerror(errno));
        fclose(bloque_a_escribir);
    }


    fclose(bloque_a_escribir);
    log_debug(logger_storage,"(escribirEnBloque) - Se escribio el contenido %s",contenido);
    return;
}




char* leerBloqueLogico(BloqueLogico* bloque_logico){
    log_debug(logger_storage, "estoy en (leerBloqueLogico)");

    FILE* arch_a_leer = fopen(bloque_logico->ptr_bloque_fisico->ruta_absoluta, "rb"); //rb
    if (!arch_a_leer){
        log_error(logger_storage, "(leerBloqueLogico) - No se pudo abrir archivo, ruta: %s",bloque_logico->ptr_bloque_fisico->ruta_absoluta);
        return NULL;
    }

    log_debug(logger_storage, "(leerBloqueLogico) pude abrir bloque a leer");


    char *contenido = malloc(datos_superblock_gb->tamanio_bloque + 1);
    fread(contenido,datos_superblock_gb->tamanio_bloque ,1 , arch_a_leer);
    fclose(arch_a_leer);

    contenido[datos_superblock_gb->tamanio_bloque] = '\0';
    log_debug(logger_storage, "(leerBloqueLogico) pude leer bloque, contenido: %s", contenido);

    return (char *)contenido;
}

//este, 1) Unlinkea bloque viejo, 2) Actualiza metadata del nuevo bloque, 3) Hardlinkear al nuevo fisico
void reapuntarBloque(BloqueFisico* bloque_fisico_a_reapuntar,BloqueLogico* bloque_logico,t_config* metadata, int query_id,char* nombre_file,char* nombre_tag){ 


    

    if (unlink(bloque_logico->ruta_hl) != 0){
        log_error(logger_storage,"(reapuntarBloque) - Pincho en unlinkear, ya fue todo loco abort");
        log_error(logger_storage,"(reapuntarBloque) - Ruta del hlink fallido: %s",bloque_logico->ruta_hl);
    } else{
         log_info(logger_storage,"## %d - %s:%s Se eliminó el hard link del bloque lógico %d al bloque físico %d",
    query_id,nombre_file,nombre_tag,bloque_logico->id_logico,bloque_logico->ptr_bloque_fisico->id_fisico);
    }
   

   
    if(crearHLink(bloque_logico->ruta_hl,bloque_fisico_a_reapuntar->ruta_absoluta)){
        log_info(logger_storage,
        "## %d - %s:%s Se agregó el hard link del bloque lógico %d al bloque físico %d",
        query_id,nombre_file,nombre_tag,bloque_logico->id_logico,bloque_fisico_a_reapuntar->id_fisico);

        log_debug(logger_storage,"(reapuntarBLoque) - Hardlink reecho");
    
    }

     char** array_blocks = config_get_array_value(metadata,"BLOCKS"); 
     array_blocks[bloque_logico->id_logico] = string_itoa(bloque_fisico_a_reapuntar->id_fisico);
   // char* bloque_a_modificar = array_blocks[bloque_logico->id_logico]; 

    log_debug(logger_storage, "(reapuntarBloque) - Pequenio chequeo antes del reapuntamiento");
    log_debug(logger_storage, "(reapuntarBloque) - Id bloque fisico en logico(estructura): %d , Bloque obtenido de la metadata(.config) %s. DEBEN SER IGUALES",
     bloque_logico->ptr_bloque_fisico->id_fisico,array_blocks[bloque_logico->id_logico]);
    //sprintf(bloque_a_modificar,"%d",bloque_fisico_a_reapuntar->id_fisico);

    char* nuevo_array_blocks = stringArrayConfigAString(array_blocks);
    config_set_value(metadata, "BLOCKS", nuevo_array_blocks);

    bloque_logico->ptr_bloque_fisico = bloque_fisico_a_reapuntar;

    config_save(metadata);

    string_array_destroy(array_blocks);
}

ErrorStorageEnum realizarELIMINAR_UN_TAG(int query_id,char *nombre_file,char *nombre_tag){ 

    if (fileInexistente(nombre_file)){
        
        return FILE_INEXISTENTE;
    }

    File *file_a_commitear = buscarFilePorNombre(nombre_file);

    if (tagInexistente(file_a_commitear->tags, nombre_tag, nombre_file)){
        
        return TAG_INEXISTENTE;
    }

    log_debug(logger_storage, "(realizarELIMINAR_UN_TAG) - file: %s, tag: %s existentes",
        nombre_file, nombre_tag);
//GENIO!!!!!!!!!!!
    Tag *tag_a_elminar = buscarTagPorNombre(file_a_commitear->tags, nombre_tag);

    log_debug(logger_storage, "(realizarELIMINAR_UN_TAG) -  tag encontrado, cantidad de bloques logicos: %d, ruta %s",
    tag_a_elminar->bloques_logicos->elements_count, tag_a_elminar->directorio);

    unlinkearBloquesLogicosParaELIMINAR_UN_TAG(query_id, list_size(tag_a_elminar->bloques_logicos), tag_a_elminar, nombre_file, nombre_tag);
    
    char* ruta_metadata_config = string_from_format("%s/metadata.config",tag_a_elminar->directorio);
    char* ruta_logical_blocks = string_from_format("%s/logical_blocks", tag_a_elminar->directorio);
    remove(ruta_metadata_config);
    
    if(rmdir(ruta_logical_blocks)){
        log_debug(logger_storage, "(realizarELIMINAR_UN_TAG) - se elimina: %s", ruta_logical_blocks);
    }
    
    if(rmdir(tag_a_elminar->directorio)){
        log_debug(logger_storage,"(realizarELIMINAR_UN_TAG) - se elimina: %s", tag_a_elminar->directorio);
    }
        

    free(ruta_logical_blocks);
    free(ruta_metadata_config);
    
    return OK;
}


void unlinkearBloquesLogicosParaELIMINAR_UN_TAG(int query_id,int cant_a_unlinkear,Tag* tag,char* nombre_file,char* nombre_tag){
    t_list* bloques_logicos = tag->bloques_logicos;
    bool esta_commiteado = estaCommiteado(tag->metadata_config_tag);
    
    for (int i = 0; i < cant_a_unlinkear; i++){
    
        BloqueLogico *bloque_popeado = (BloqueLogico *)list_remove(bloques_logicos, i); //agregamos el -1
        BloqueFisico *bloque_fisico_asociado = bloque_popeado->ptr_bloque_fisico;

        //---------------------------------- /files/tag/logical-blocks/0002.dat
        log_debug(logger_storage, "(unlinkearBloquesLogicosParaELIMINAR_UN_TAG) - hlink a unlinkear: %s", bloque_popeado->ruta_hl);
        unlink(bloque_popeado->ruta_hl);       // quitarle el hlink
        log_info(logger_storage,"## %d - %s:%s Se eliminó el hard link del bloque lógico %d al bloque físico %d",
        query_id,nombre_file,nombre_tag,bloque_popeado->id_logico,bloque_popeado->ptr_bloque_fisico->id_fisico);
        

        if (!tieneHLinks(bloque_fisico_asociado->ruta_absoluta)){ // 1 hard links -> liberar en el bitmap
            log_debug(logger_storage,"(unlinkearBloquesLogicosParaELIMINAR_UN_TAG) - no tenia hrlinks");
            liberarBloqueDeBitmap(bloque_fisico_asociado->id_fisico, query_id);
            
            if(esta_commiteado){
                DatosParaHash* datos_para_hash = obtenerDatosParaHash(bloque_popeado);
                char* hash_a_remover = crypto_md5(datos_para_hash->contenido,datos_para_hash->tamanio);      
                config_remove_key(hash_index_config_gb,hash_a_remover);

                config_save(hash_index_config_gb);
                liberarDatosParaHash(datos_para_hash);
                free(hash_a_remover);
            }

        
            
            log_debug(logger_storage, "(unlinkearBloquesLogicosParaELIMINAR_UN_TAG)- El bloque fisico %s fue liberado", bloque_fisico_asociado->nombre);
        }
        else{
            log_debug(logger_storage, "(unlinkearBloquesLogicosParaELIMINAR_UN_TAG) - tenia hlinks");
        }
        

        //bloque_popeado->ptr_bloque_fisico = NULL; // JORGE EL CURIOSO - Capaz por enunciado deberia apuntar al block0;
        liberarBloqueLogico(bloque_popeado);
        log_debug(logger_storage, "(unlinkearBloquesLogicos) - FER lo hizo de nuevo");
    }
}


bool estaCommiteado(t_config* metadata){
    char* estado = config_get_string_value(metadata, "ESTADO");
    return string_equals_ignore_case(estado, "COMMITED");
}

/*
void esperar_operacion() {
    if (retardo_operacion > 0)
        usleep(retardo_operacion * 1000); // ms --> µs
}

void esperar_acceso_bloque() {
    if (retardo_acceso_bloque > 0)
        usleep(retardo_acceso_bloque * 1000); // ms --> µs
}
*/

/*
mutex storage : 
pthread_mutex_lock(&mutex_files);
pthread_mutex_lock(&mutex_bitmap);

*/
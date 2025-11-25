#include "operaciones.h"

void *recursosHumanos(void *args_sin_formato)
{
    int socket_cliente = *(int *)args_sin_formato;

    while (1)
    {
        int fd_cliente;
        fd_cliente = esperarCliente(socket_cliente, logger_storage);

        pthread_t thread_labubu;
        pthread_create(&thread_labubu, NULL, atenderLaburanteDisconforme, (void *)&fd_cliente);
        pthread_detach(thread_labubu);
    } // n veces porque hay n workers
}

void *atenderLaburanteDisconforme(void *args_sin_formato)
{
    int fd_cliente = *((int *)args_sin_formato);

    handshake(fd_cliente);

    do
    {
        pedidoDeLaburante(fd_cliente);
    } while (1);
}

void pedidoDeLaburante(int mail_laburante)
{

    Mensaje *mensajito;
    mensajito = recibirMensajito(mail_laburante);

    log_debug(logger_storage,
              "Debug - (pedidoDeLaburante) - Recibi el mensaje %s", mensajito->mensaje);

    // COD_OP QUERY_ID ...
    char **mensajito_cortado = string_split(mensajito->mensaje, " ");
    CodOperacionStorage tipo_operacion = obtenerTareaCodOperacion(mensajito_cortado[0]);
    char *query_id = mensajito_cortado[1];
    hacerRetardoOperacion();

    switch (tipo_operacion)
    {

    case CREATE:
        // CREATE QUERY_ID NOMBRE_FILE TAG
        char *nombre_file = mensajito_cortado[2]; // TODO MANY A LA OBRA
        char *tag = mensajito_cortado[3];

        int resultado_CREATE = realizarCREATE(query_id, nombre_file, tag);
        if (resultado_CREATE != OK)
        {
            enviarMensajito(mensajitoError(FILE_PREEXISTENTE), mail_laburante, logger_storage);
            log_error(logger_storage, "NO SE PUDO REALIZAR CREATE POR MOTIVO FILE_PREEXISTENTE"); // JOE PINO
        }
        string_array_destroy(mensajito_cortado);
        break;

    case TRUNCATE:
        // TRUNCATE QUERY_ID FILE:TAG TAMANIO
        //----FILE:TAG
        char *nombre_full_file_truncate = mensajito_cortado[2];
        int tamanio_a_truncar = atoi(mensajito_cortado[3]);

        int resultado = realizarTRUNCATE(query_id, nombre_full_file_truncate, tamanio_a_truncar);
        if (resultado != OK)
        {
            enviarMensajito(mensajitoError(resultado), mail_laburante, logger_storage);
            log_error(logger_storage, "NO SE PUDO REALIZAR TRUNCATE POR MOTIVO %s", NOMBRE_ERRORES[resultado]); // JOE PINO
        }

        string_array_destroy(mensajito_cortado);
        break;

    case TAG:
        // TAG QUERY_D FILE_O:TAG_O FILE_D:TAG_D
        char *nombre_completo_origen = mensajito_cortado[2];
        char *nombre_completo_destino = mensajito_cortado[3];
        enviarMensajito(mensajitoOk(), mail_laburante, logger_storage);

        int resultado_tag = realizarTAG(query_id, nombre_completo_origen, nombre_completo_destino);
        if (resultado_tag != OK)
        {
            enviarMensajito(mensajitoError(resultado_tag), mail_laburante, logger_storage);
            log_error(logger_storage, "NO SE PUDO REALIZAR TAG POR MOTIVO %s", NOMBRE_ERRORES[resultado_tag]);
        }

        string_array_destroy(mensajito_cortado);

        break;

    case COMMIT:
        // COMMIT QUERY_ID FILE:TAG
        char *nombre_completo_commit = mensajito_cortado[2];

        int resultado_commit = realizarCOMMIT(query_id,nombre_completo_commit);
        if (resultado_commit != OK)
        {
            enviarMensajito(mensajitoError(resultado_commit), mail_laburante, logger_storage);
            log_error(logger_storage, "NO SE PUDO REALIZAR COMMIT POR MOTIVO %s", NOMBRE_ERRORES[resultado_commit]);
        }
        log_info(logger_storage, "## %d - Commit de File:Tag %s", query_id, nombre_completo_commit);
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

bool realizarCREATE(char *query_id, char *nombre_file, char *nombre_tag)
{

    if (filePreexistente(nombre_file))
        return FILE_PREEXISTENTE; // chequeo de error del enunciado

    File *nuevo_file = crearFile(nombre_file);
    list_add(lista_files_gb, nuevo_file);
    Tag *nuevo_tag = crearTag(nombre_tag, nombre_file);

    list_add(nuevo_file->tags, nuevo_tag);

    log_info(
        logger_storage, "## %s - File Creado %s:%s",
        query_id, nombre_file, nombre_tag);

    return OK;
}

ErrorStorageEnum realizarTRUNCATE(int query_id, char *file_completo, int tamanio_a_truncar)
{
    char *nombre_file;
    char *nombre_tag;

    // probar funcionamiento
    asignarFileTagAChars(nombre_file, nombre_tag, file_completo);

    // chequeo errores
    pthread_mutex_lock(&mutex_files);
    if (fileInexistente(nombre_file))
    {
        free(nombre_file);
        free(nombre_tag);
        pthread_mutex_unlock(&mutex_files);
        return FILE_INEXISTENTE;
    }

    File *file = buscarFilePorNombre(nombre_file);
    if (tagInexistente(file->tags, nombre_tag, nombre_file))
    {
        free(nombre_file);
        free(nombre_tag);
        pthread_mutex_unlock(&mutex_files);
        return TAG_INEXISTENTE;
    }

    Tag *tag_concreto = buscarTagPorNombre(file->tags, nombre_tag);

    gestionarTruncateSegunTamanio(tag_concreto, tamanio_a_truncar);
    actualizarTamanioMetadata(nombre_file, tag_concreto, tamanio_a_truncar);

    pthread_mutex_unlock(&mutex_files);
    free(nombre_file);
    free(nombre_tag);

    return OK;
}

void hacerRetardoOperacion()
{
    sleep(config_storage->retardo_operacion / 1000); // te tengo al lado y me siento solo el miedo me come y no entiendo como raszones no faltan para que me
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
    File *nuevo_file = malloc(sizeof(File));
    nuevo_file->nombre_file = string_duplicate(nombre_file); // LIBERAR
    char *dir_a_crear;
    sprintf(dir_a_crear, "%s/%s", RUTA_FILES, nuevo_file->nombre_file); // asignas la ruta abs del file

    crearDirectorio(dir_a_crear); // mkdir como Agustin Coda

    nuevo_file->tags = list_create();
}

Tag *crearTag(char *nombre_tag, char *nombre_file_asociado)
{
    Tag *tag = malloc(sizeof(Tag));

    tag->nombre_tag = string_duplicate(nombre_tag); // LIBERAR

    // home/utnso/rerewr/reewrwe/ewr/wer/nombrefile/nombretag
    sprintf(tag->directorio, "%s/%s/%s", RUTA_FILES, nombre_file_asociado, tag->nombre_tag);
    crearDirectorio(tag->directorio);

    // Creo carpeta logical blocks
    char *ruta_aux_lblocks;
    sprintf(ruta_aux_lblocks, "%s/logical_blocks", tag->directorio);
    crearDirectorio(ruta_aux_lblocks);

    tag->metadata_config_tag = crearMetadata(tag->directorio);

    tag->bloques_logicos = list_create();

    log_debug(logger_storage, "Debug - (crearTag) - Se creo el tag %s", nombre_tag);

    return tag;
}

t_config *crearMetadata(char *path_tag)
{
    char *path_metadata;
    sprintf(path_metadata, "%s/metadata.config", path_tag);

    FILE *arch_config = fopen(path_metadata, "w+");
    fclose(arch_config);

    t_config *metadata = config_create(path_metadata);
    config_set_value(metadata, "TAMANIO", "0");
    config_set_value(metadata, "BLOCKS", "[]");
    config_set_value(metadata, "ESTADO", "WORK_IN_PROGRESS");

    config_save_in_file(metadata, path_metadata);
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
        return (strcmp(nombre, file->nombre_file));
    }
    return list_find(lista_files_gb, tieneMismoNombre);
}

Tag *buscarTagPorNombre(t_list *tags, char *nombre_tag)
{

    bool tieneMismoNombreTag(void *ptr)
    {
        Tag *tag = (Tag *)ptr;
        return (strcmp(nombre_tag, tag->nombre_tag));
    }

    return list_find(tags, tieneMismoNombreTag);
}

void gestionarTruncateSegunTamanio(Tag *tag_concreto, int tamanio_a_truncar)
{
    int tamanio_actual = config_get_int_value(tag_concreto->metadata_config_tag, "TAMANIO");

    if (tamanio_actual == tamanio_a_truncar)
    {
        log_debug(logger_storage, "DEBUG -(gestionarTruncateSegunTamanio)- Mismo nuevo tamanio recibido en TRUNCATE");
        return true;
    }
    else if (tamanio_actual > tamanio_a_truncar)
    {
        ferConLaMexicana(tag_concreto, tamanio_actual, tamanio_a_truncar); // siempre te podes achicar, sino miralo a Fer
        return true;
    }
    else
    {
        return agrandarEnTruncate(tag_concreto, tamanio_actual, tamanio_a_truncar);
    }
}

void ferConLaMexicana(Tag *tag, int tamanio_actual, int nuevo_tamanio)
{ // tag tamanio
    int cant_bloques_a_desasignar = (tamanio_actual - nuevo_tamanio) / datos_superblock_gb->tamanio_bloque;
    unlinkearBloquesLogicos(cant_bloques_a_desasignar, tag->bloques_logicos);
}

void unlinkearBloquesLogicos(int cant_a_unlinkear, t_list *bloques_logicos){
    for (int i = 0; i < cant_a_unlinkear; i++){
        BloqueLogico *bloque_popeado = (BloqueLogico *)list_remove(bloques_logicos, list_size(bloques_logicos));
        BloqueFisico *bloque_fisico_asociado = bloque_popeado->ptr_bloque_fisico;

        //---------------------------------- /files/tag/logical-blocks/0002.dat
        unlink(bloque_popeado->ruta_hl);       // quitarle el hlink
        bloque_popeado->ptr_bloque_fisico = NULL; // JORGE EL CURIOSO - Capaz por enunciado deberia apuntar al block0;

        if (!tieneHLinks(bloque_fisico_asociado->ruta_absoluta)){ // 1 hard links -> liberar en el bitmap
            liberarBloqueDeBitmap(bloque_fisico_asociado->id_fisico);

            log_debug(logger_storage, "(unlinkearBloquesLogicos)- El bloque fisico %s fue liberado", bloque_fisico_asociado->nombre);
        }

        liberarBloqueLogico(bloque_popeado);
        log_debug(logger_storage, "(unlinkearBloquesLogicos) - FER lo hizo de nuevo");
    }
}

bool agrandarEnTruncate(Tag *tag, int tamanio_acutal, int nuevo_tamanio)
{
    int tamanio_a_agrandar = nuevo_tamanio - tamanio_acutal;
    int cant_bloques_necesarios = tamanio_a_agrandar / datos_superblock_gb->tamanio_bloque;

    asignarBloquesFisicosATag(tag, cant_bloques_necesarios);
    // actualizar metadata
    config_set_value(tag->metadata_config_tag, "TAMANIO", itoa(nuevo_tamanio));
    config_save(tag->metadata_config_tag);

    return true;
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

    config_set_value(tag_a_asignar_hardlinks->metadata_config_tag, "BLOCKS", nueva_info_blocks_metadata);
    config_save(tag_a_asignar_hardlinks->metadata_config_tag);

    free(nueva_info_blocks_metadata);
}

// TODO - PROXIMAMENTE EN DBZ - arreglar bien tema directorios en bloque logico y tag
// ----------------------/home/utnso/storage/files/tag1_0_0
BloqueLogico *crearBloqueLogico(int nro_bloque_logico, BloqueFisico *bloque_fisico_a_asignar, char *path_tag)
{ //-- ta checkkkk  -- update 22/11/25 que hijo de puta, firma Joe
    BloqueLogico *bloque_logico = malloc(sizeof(BloqueLogico));
    bloque_logico->ptr_bloque_fisico = bloque_fisico_a_asignar;
    bloque_logico->id_logico = nro_bloque_logico;

    sprintf(bloque_logico->ruta_hl, "%s/logical_blocks/%04d.dat", path_tag, nro_bloque_logico);

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
    FILE *dat_a_crear = fopen(ruta_hl_del_bloque_logico, "a+"); // logical block a crear
    fclose(dat_a_crear);
    if (!link(bloque_fisico_a_hardlinkear, ruta_hl_del_bloque_logico))
        return false;
    return true;
}

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

void liberarBloqueDeBitmap(int nro_bloque)
{

    bitarray_clean_bit(bitmap_gb, nro_bloque);
    log_debug(logger_storage, "Debug - (limpiarBitsPorStringArray) - Se libero del bitmap el bloque nro %d", nro_bloque);
}

char *obtenerNombreBloqueConCeros(int numero)
{
    char *nombre;
    // NANO
    sprintf(nombre, "%04d", numero);
    return nombre;
}

void liberarBloqueLogico(BloqueLogico *bloque_a_liberar)
{
    // free(bloque_a_liberar->directorio);
    // free(bloque_a_liberar->nombre);
    free(bloque_a_liberar);
}

bool tieneHLinks(char *ruta_abs_a_consultar)
{
    struct stat info;
    if (stat(ruta_abs_a_consultar, &info) == -1)
    {
        log_debug(logger_storage, "Debug -(contarHLinks)- Error en stat");
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
    string_array_destroy(array_a_pasar_a_string);
    return string_a_retornar;
}

ErrorStorageEnum realizarTAG(int query_id, char *nombre_origen_completo, char *nombre_destino_completo)
{

    char *nombre_file_origen;
    char *tag_origen;

    char *nombre_file_destino;
    char *tag_destino;

    // probar funcionamiento
    asignarFileTagAChars(nombre_file_origen, tag_origen, nombre_origen_completo); // TODO

    // ======================== chequeo errores
    pthread_mutex_lock(&mutex_files);
    // NO EXISTE FILE ORIGEN
    if (fileInexistente(nombre_file_origen))
    {
        free(nombre_file_origen);
        free(tag_origen);
        pthread_mutex_unlock(&mutex_files);
        return FILE_INEXISTENTE;
    }

    File *file_origen = buscarFilePorNombre(nombre_file_origen);
    // NO EXISTE TAG ORIGEN
    if (tagInexistente(file_origen->tags, tag_origen, nombre_file_origen))
    {
        free(nombre_file_origen);
        free(tag_origen);
        pthread_mutex_unlock(&mutex_files);
        return TAG_INEXISTENTE;
    }
    asignarFileTagAChars(nombre_file_destino, tag_destino, nombre_destino_completo);

    // CASOS
    // file creado
    // file tag creado
    // ninguno creado

    // NINGUNO CREADO

    if (!filePreexistente(nombre_file_destino))
    {
        File *file_destino_creado = crearFile(nombre_file_destino);
        Tag *tag_destino_creado = crearTag(tag_destino, nombre_file_destino);
        asignarBloquesFisicosATagCopiado(tag_destino_creado);

        pthread_mutex_unlock(&mutex_files);
        free(nombre_file_origen);
        free(tag_origen);
        free(nombre_file_destino);
        free(tag_destino);
        return OK;
    }

    // FILE CREADO, TAG INEXISTENTE
    File *file_destino = buscarFilePorNombre(nombre_file_destino);
    if (!tagPreexistente(file_destino->tags, tag_destino, nombre_file_destino))
    {
        Tag *tag_destino_a_crear = crearTag(tag_destino, nombre_file_destino);
        asignarBloquesFisicosATagCopiado(tag_destino_a_crear);

        pthread_mutex_unlock(&mutex_files);
        free(nombre_file_origen);
        free(tag_origen);
        free(nombre_file_destino);
        free(tag_destino);
        log_info(logger_storage, "## %d - Tag creado %s", query_id, nombre_destino_completo);
        return OK;
    }

    // TODO CREADO, SOLO COPIA
    Tag *tag_destino_encontrado = buscarTagPorNombre(file_origen->tags, tag_origen);
    asignarBloquesFisicosATagCopiado(tag_destino);

    return OK;
}

int copiarTag(Tag *tag_origen, Tag *tag_destino)
{

    char *aux_path_metadata_destino;
    //------------------------------- PROBAR  SI REALMENTE LO COPIA EN EL DESTINO
    sprintf(aux_path_metadata_destino, "%s/metadata.config", tag_destino->directorio);

    config_save_in_file(tag_origen->metadata_config_tag, aux_path_metadata_destino);
    config_set_value(tag_destino, "ESTADO", "WORK_IN_PROGRESS");

    //------------------------------- tag_destino ---------------> [10,2,0,0,0,0]
    asignarBloquesFisicosATagCopiado(tag_destino);
}

void asignarBloquesFisicosATagCopiado(Tag *tag_destino)
{
    char **bloques_logicos_a_copiar = config_get_array_value(tag_destino->metadata_config_tag, "BLOCKS");
    t_list *logicos_a_copiar = tag_destino->bloques_logicos;

    if (!list_is_empty(logicos_a_copiar))
    { // caso donde el tag estaba creado y CAPAZ tenia blogicos asignados
        log_debug(logger_storage, "(asignarBloquesFisicosATagCopiado) - El tag %s destino tenia cosas, se liberaron");
        unlinkearBloquesLogicos(list_size(logicos_a_copiar), logicos_a_copiar);
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

BloqueFisico *obtenerBloqueFisico(int nro_bloque_a_buscar)
{
    return (BloqueFisico *)list_get(bloques_fisicos_gb, nro_bloque_a_buscar);
}

ErrorStorageEnum realizarCOMMIT(char* query_id,char *nombre_completo){
    char *nombre_file;
    char *nombre_tag;

    asignarFileTagAChars(nombre_file, nombre_tag, nombre_completo);

    if (fileInexistente(nombre_file))
    {
        free(nombre_file);
        free(nombre_tag);
        return FILE_INEXISTENTE;
    }
    File *file_a_commitear = buscarFilePorNombre(nombre_file);

    if (tagInexistente(file_a_commitear->tags, nombre_tag, nombre_file))
    {
        free(nombre_file);
        free(nombre_tag);
        return TAG_INEXISTENTE;
    }

    Tag *tag_a_commitear = buscarTagPorNombre(file_a_commitear->tags, nombre_tag);

    if (tieneEstadoCOMMITED(tag_a_commitear))
    {
        log_debug(logger_storage, "(realizarCOMMIT) - El estado del tag %s ya estaba en COMMITED", tag_a_commitear->nombre_tag);
        free(nombre_file);
        free(nombre_tag);
        return OK;
    }

    int cant_blogicos = list_size(tag_a_commitear->bloques_logicos);
    escribirEnHashIndex(tag_a_commitear);
    log_info(logger_storage,"## %s - Commit de File:Tag %s",query_id,nombre_completo);
    free(nombre_file);
    free(nombre_tag);
    return OK;
}
/*
Bloques Logicos. Cada uno apunta a un bloque fisico.
Commitear: 5: cambie 2
*/


void escribirEnHashIndex(Tag *tag){
    int cant_blogicos = list_size(tag->bloques_logicos);
    
    for (int i = 0; i < cant_blogicos; i++)
    {
        BloqueLogico *bloque_logico_a_consultar = list_get(tag->bloques_logicos, i);
        DatosParaHash *datos_para_hash = obtenerDatosParaHash(bloque_logico_a_consultar);

        char *hash_bloque_logico = crypto_md5(datos_para_hash->contenido, datos_para_hash->tamanio); // block0000 // 9
        bool existe_hash = config_has_property(hash_index_config_gb,hash_bloque_logico);
        
        if (existe_hash){
            log_debug(logger_storage,"(escribirEnHashIndex) - Hash encontrado en el index config, tratando de reapuntar");
            char* bloque_fisico_de_config = string_duplicate(config_get_string_value(hash_index_config_gb,hash_bloque_logico));
            
            BloqueFisico* bloque_fisico_a_apuntar = buscarBloquePorNombre(bloque_fisico_de_config);
            reapuntarBloque(bloque_fisico_a_apuntar,bloque_logico_a_consultar);

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
        return (strcmp(nombre, bloque->ruta_absoluta));
    }
    return list_find(bloques_fisicos_gb, tieneMismoNombre);
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

void *obtenerContenidoBloqueFisico(BloqueLogico *bloque_logico, int tamanio){

    FILE *arch_a_leer = fopen(bloque_logico->ptr_bloque_fisico->ruta_absoluta, "rw");
    if (!arch_a_leer)
        abort();

    void *contenido = malloc((size_t)tamanio);
    fread(contenido, 1, (size_t)tamanio, arch_a_leer);
    fclose(arch_a_leer);

    return contenido;
}

//este, 1) Unlinkea bloque viejo, 2) Actualiza metadata del nuevo bloque, 3) Hardlinkear al nuevo fisico
void reapuntarBloque(BloqueFisico* bloque_fisico_a_reapuntar,BloqueLogico* bloque_logico,t_config* metadata){ 

    if (!unlink(bloque_logico->ruta_hl)){
        log_error(logger_storage,"(reapuntarBloque) - Pincho en unlinkear, ya fue todo loco abort");
        log_error(logger_storage,"(reapuntarBloque) - Ruta del hlink fallido: %s",bloque_logico->ruta_hl);
        abort();
    }

    char** array_blocks = config_get_array_value(metadata,"BLOCKS"); 
    char* bloque_a_modificar = array_blocks[bloque_logico->id_logico]; 

    log_debug(logger_storage, "(reapuntarBloque) - Pequenio chequeo antes del reapuntamiento");
    log_debug(logger_storage, "(reapuntarBloque) - Id bloque fisico en logico(estructura): %d , Bloque obtenido de la metadata(.config) %s. DEBEN SER IGUALES", bloque_logico->ptr_bloque_fisico->id_fisico,bloque_a_modificar);
    sprintf(bloque_a_modificar,"%d",bloque_fisico_a_reapuntar->id_fisico);

    char* nuevo_array_blocks = stringArrayConfigAString(array_blocks);
    config_set_value(metadata, "BLOCKS", nuevo_array_blocks);

    bloque_logico->ptr_bloque_fisico = bloque_fisico_a_reapuntar;

    crearHLink(bloque_logico->ruta_hl,bloque_fisico_a_reapuntar->ruta_absoluta);
    log_debug(logger_storage,"(reapuntarBLoque) - Hardlink reecho");
}

ErrorStorageEnum realizarELIMINAR_UN_TAG(char* query_id,char* nombre_completo){ 
    char *nombre_file;
    char *nombre_tag;

    asignarFileTagAChars(nombre_file, nombre_tag, nombre_completo);

    if (fileInexistente(nombre_file)){
        free(nombre_file);
        free(nombre_tag);
        return FILE_INEXISTENTE;
    }

    File *file_a_commitear = buscarFilePorNombre(nombre_file);

    if (tagInexistente(file_a_commitear->tags, nombre_tag, nombre_file)){
        free(nombre_file);
        free(nombre_tag);
        return TAG_INEXISTENTE;
    }

    Tag *tag_a_commitear = buscarTagPorNombre(file_a_commitear->tags, nombre_tag);

    unlinkearBloquesLogicosELIMINAR_UN_TAG(list_size(tag_a_commitear->bloques_logicos), tag_a_commitear->bloques_logicos);
    char* ruta_metadata_config;
    
    sprintf(ruta_metadata_config, "%s/metadata.config",tag_a_commitear->directorio);
    remove(ruta_metadata_config);
    rmdir(tag_a_commitear->directorio);
    log_info(logger_storage,"## %s - Tag Eliminado %s",query_id,nombre_completo);
    
    free(nombre_file);
    free(nombre_tag);
}



void unlinkearBloquesLogicosParaELIMINAR_UN_TAG(int cant_a_unlinkear,Tag* tag){
    t_list* bloques_logicos = tag->bloques_logicos;
    bool esta_commiteado = estaCommiteado(tag->metadata_config_tag);
    
    for (int i = 0; i < cant_a_unlinkear; i++){
        BloqueLogico *bloque_popeado = (BloqueLogico *)list_remove(bloques_logicos, list_size(bloques_logicos));
        BloqueFisico *bloque_fisico_asociado = bloque_popeado->ptr_bloque_fisico;

        //---------------------------------- /files/tag/logical-blocks/0002.dat
        unlink(bloque_popeado->ruta_hl);       // quitarle el hlink
        bloque_popeado->ptr_bloque_fisico = NULL; // JORGE EL CURIOSO - Capaz por enunciado deberia apuntar al block0;

        if (!tieneHLinks(bloque_fisico_asociado->ruta_absoluta)){ // 1 hard links -> liberar en el bitmap
            liberarBloqueDeBitmap(bloque_fisico_asociado->id_fisico);
            
            if(esta_commiteado){
                DatosParaHash* datos_para_hash = obtenerDatosParaHash(bloque_popeado);
                char* hash_a_remover = crypto_md5(datos_para_hash->contenido,datos_para_hash->tamanio);      
                config_remove_key(hash_index_config_gb,hash_a_remover);

                config_save(hash_index_config_gb);
                liberarDatosParaHash(datos_para_hash);
                free(hash_a_remover);
            }
            
            log_debug(logger_storage, "(unlinkearBloquesLogicos)- El bloque fisico %s fue liberado", bloque_fisico_asociado->nombre);
        }

        liberarBloqueLogico(bloque_popeado);
        log_debug(logger_storage, "(unlinkearBloquesLogicos) - FER lo hizo de nuevo");
    }
}


bool estaCommiteado(t_config* metadata){
    return string_contains(config_get_string_value(metadata, "ESTADO"), "COMMITED");
}

#include "ciclo_instruccion.h"
#include <stdint.h>

t_instruccion *instruccion;
int socket_master;
int socket_storage;
char* NOMBRE_OPERACIONES[9] = 
    {"CREATE","TRUNCATE","WRITE","READ","TAG","COMMIT","FLUSH","DELETE","END"};
int CANT_OPERACIONES_WORKER = 9;



/*t_list *crearListaDeInstrucciones(){
    if (!query || !query->nombre) {
        log_error(logger_worker, "Query o nombre nulo al cargar instrucciones");
        return NULL;
    }

    char *path_completo = string_from_format("%s/%s",config_worker->path_queries, query->nombre);
    FILE *file_query = fopen(path_completo, "r");
    

    if (!file_query){
        log_error(logger_worker, "(crearListaDeInstrucciones) - Error al abrir el archivo en la ruta %s",path_completo);
        free(path_completo);
        return NULL;
    }

    fscanf(file_query, "%*s");

    t_list *lista_instucciones = list_create();
    char *buffer = NULL;
    size_t tam = 0;
    ssize_t leidos;

    

    while ((leidos = getline(&buffer, &tam, file_query)) != -1)
    {
        // Sacar el salto de línea si lo tiene
        if (leidos > 0 && buffer[leidos - 1] == '\n')
            buffer[leidos - 1] = '\0';

        // strdup para copiar, porque getline reutiliza el buffer
        char *linea = strdup(buffer);
        if (!linea){
            log_error(logger_worker, "(crearListaDeInstrucciones) Fallo strdup → memoria agotada");
            free(buffer);
            fclose(file_query);
            free(path_completo);
            list_destroy_and_destroy_elements(lista_instucciones, free);
            return NULL;
        }
        list_add(lista_instucciones, linea);
    }

    free(buffer);
    fclose(file_query);
    free(path_completo);
    

    log_info(logger_worker, "Query %d: Cargadas %d instrucciones", query->id_query, list_size(lista_instucciones));

    return lista_instucciones;
}*/

t_list *crearListaDeInstrucciones(){
    if (!query || !query->nombre) {
        log_error(logger_worker, "Query o nombre nulo al cargar instrucciones");
        return NULL;
    }

    char *path_completo = string_from_format("%s/%s", config_worker->path_queries, query->nombre);
    FILE *file_query = fopen(path_completo, "r");
    
    if (!file_query){
        log_error(logger_worker, "(crearListaDeInstrucciones) - Error al abrir el archivo en la ruta %s", path_completo);
        free(path_completo);
        return NULL;
    }

    t_list *lista_instucciones = list_create();
    char *buffer = NULL;
    size_t tam = 0;
    ssize_t leidos;

    while ((leidos = getline(&buffer, &tam, file_query)) != -1)
    {
        // Sacar el salto de línea si lo tiene
        if (leidos > 0 && buffer[leidos - 1] == '\n')
            buffer[leidos - 1] = '\0';

        // Saltar líneas vacías
        if (strlen(buffer) == 0)
            continue;

        // strdup para copiar, porque getline reutiliza el buffer
        char *linea = strdup(buffer);
        if (!linea){
            log_error(logger_worker, "(crearListaDeInstrucciones) Fallo strdup → memoria agotada");
            free(buffer);
            fclose(file_query);
            free(path_completo);
            list_destroy_and_destroy_elements(lista_instucciones, free);
            return NULL;
        }
        list_add(lista_instucciones, linea);
    }

    free(buffer);
    fclose(file_query);
    free(path_completo);
    
    log_info(logger_worker, "Query %d: Cargadas %d instrucciones", query->id_query, list_size(lista_instucciones));

    return lista_instucciones;
}
void destruir(void *element) { free(element); }

char* Fetch(){

    char *instruccion = list_get(query->instrucciones, query->pc_query);

    if(!instruccion){
        log_debug(logger_worker,"(Fetch)- No hay una instruccion en ese PC.");
        return NULL;
    }

    char** instruccion_separada = string_split(instruccion, " ");
    char* op_code = instruccion_separada[0];

    log_info(logger_worker,"## Query %d: FETCH - Program Counter: %d - %s", query->id_query, query->pc_query, op_code);

    return instruccion;
}

int obtenerOperacionCodOp(char* string_operacion){
    for (int i = 0; i < CANT_OPERACIONES_WORKER; i++){
        
        if (strcmp(NOMBRE_OPERACIONES[i],string_operacion)==0)
		return i;
    
    }
    return -1;
}

void Decode(char* instruccionCom){

    char **instruccion_separada = string_split(instruccionCom, " ");
    // [cod-op,cola]

    char *op_code_string = instruccion_separada[0];
    instruccion->operacion = instruccion_separada[1];
    instruccion->cod_op = obtenerOperacionCodOp(op_code_string);
    /*if (strcmp(op_code, "CREATE") == 0)
        instruccion->cod_op = CREATE;
    else if (strcmp(op_code, "TRUNCATE") == 0)
        instruccion->cod_op = TRUNCATE;
    else if (strcmp(op_code, "READ") == 0)
        instruccion->cod_op = READ;
    else if (strcmp(op_code, "WRITE") == 0)
        instruccion->cod_op = WRITE;
    else if (strcmp(op_code, "TAG") == 0)
        instruccion->cod_op = TAG;
    else if (strcmp(op_code, "COMMIT") == 0)
        instruccion->cod_op = COMMIT;
    else if (strcmp(op_code, "FLUSH") == 0)
        instruccion->cod_op = FLUSH;
    else if (strcmp(op_code, "DELETE") == 0)
        instruccion->cod_op = DELETE;
    else if (strcmp(op_code, "END") == 0)
        instruccion->cod_op = END;*/
}

bool Execute(){

    char* nombre_instruccion; // create 1:rrere
    char* file;
    char* tag;
    bool finaliza = true;
    if (instruccion->cod_op != END){ //TRUNCATE <NOMBRE_FILE>:<TAG> <TAMAÑO>

        file = strtok(instruccion->operacion, ":");
        tag = strtok(NULL, " ");
        finaliza = false;
    }

    switch (instruccion->cod_op){

    case CREATE:
        //<NOMBRE_FILE>:<TAG>
        log_debug(logger_worker, "(Execute) - Ejecutando CREATE %s:%s", file, tag);
        finaliza = ejecutarCreate(file, tag);
        nombre_instruccion = "CREATE";
        // tipo_instruccion = "REQUIERO_DESALOJO";
        break;

    case TRUNCATE:
        //<NOMBRE_FILE>:<TAG> <TAMAÑO>
        int tamanio_truncate = atoi(strtok(NULL, " "));
        log_debug(logger_worker, "(Execute) - Ejecutando TRUNCATE %s:%s TAMANIO:%d", file, tag, tamanio_truncate);
        
        finaliza = ejecutarTruncate(file, tag, tamanio_truncate);
        nombre_instruccion = "TRUNCATE";
        // tipo_instruccion = "REQUIERO_DESALOJO";
        break;

    case WRITE:
        //<FILE>:<TAG> <DIRECCIÓN BASE> <CONTENIDO>
        log_debug(logger_worker, "(Execute) - Ejecutando WRITE %s:%s", file, tag);
        int dir_base_write = atoi(strtok(NULL, " "));
        char *contenido = strtok(NULL, " ");

        finaliza = ejecutarWrite(file, tag, dir_base_write, contenido);
        nombre_instruccion = "WRITE";
        break;

    case READ:
        //<NOMBRE_FILE>:<TAG> <DIRECCIÓN BASE> <TAMAÑO>
        log_debug(logger_worker, "(Execute) - Ejecutando READ %s:%s", file, tag);
        int dir_base_read = atoi(strtok(NULL, " "));
        int tamanio_read = atoi(strtok(NULL, " "));

        finaliza = ejecutarRead(file, tag, dir_base_read, tamanio_read);
        nombre_instruccion = "READ";
        break;

    case TAG:
        //<NOMBRE_FILE_ORIGEN>:<TAG_ORIGEN> <NOMBRE_FILE_DESTINO>:<TAG_DESTINO
        log_debug(logger_worker, "(Execute) - Ejecutando TAG %s:%s", file, tag);
        char *file_destino = strtok(NULL, ":");
        char *tag_destino = strtok(NULL, " ");

        finaliza = ejecutarTag(file, tag, file_destino, tag_destino);
        nombre_instruccion = "TAG";
        break;

    case COMMIT:
        //<NOMBRE_FILE>:<TAG>
        log_debug(logger_worker, "(Execute) - Ejecutando COMMIT %s:%s", file, tag);
        finaliza = ejecutarCommit(file, tag);
        nombre_instruccion = "COMMIT";
        break;

    case FLUSH:
        //<NOMBRE_FILE>:<TAG>
        log_debug(logger_worker, "(Execute) - Ejecutando FLUSH %s:%s", file, tag);
        finaliza = ejecutarFlush(file, tag);
        nombre_instruccion = "FLUSH";
        break;

    case DELETE:
        //<NOMBRE_FILE>:<TAG>
        log_debug(logger_worker, "(Execute) - Ejecutando DELETE %s:%s", file, tag);
        finaliza = ejecutarDelete(file, tag);
        nombre_instruccion = "DELETE";
        break;

    case END:
        log_debug(logger_worker, "(Execute) - Ejecutando END");
        finaliza = ejecutarEnd();
        nombre_instruccion = "END";
        return finaliza;

    default:
        log_error(logger_worker,"Error - (Execute) - ingrese una instruccion valida");
        return true;
    }


    log_info(logger_worker,"## Query %d: - Instrucción realizada: %s", query->id_query, nombre_instruccion);
    
    return finaliza;
}



bool ejecutarCreate(char *file, char *tag){
    // faltaria crear alguna funcion para confirmar que se haya creado el tag para ese file (maybe)
    char *mensaje_creacion = string_from_format("CREATE %d %s %s",query->id_query,file,tag);
    
    pthread_mutex_lock(&sem_instruccion);
    Mensaje* mensajito = crearMensajito(mensaje_creacion);
    
    enviarMensajito(mensajito,socket_storage,logger_worker);
    Mensaje* resp_create = recibirMensajito(socket_storage,logger_worker);
    if (!string_equals_ignore_case(resp_create->mensaje,"OK")){

        log_debug(logger_worker, "(ejecutarCreate) Query %d finalizada", query->id_query);
        
        char* formato_error_master = string_from_format("ERROR %s", resp_create->mensaje);
        Mensaje* mensaje_error_master = crearMensajito(formato_error_master);
        enviarMensajito(mensaje_error_master, socket_master ,logger_worker);
        

        liberarMensajito(resp_create);
        free(formato_error_master);
        return true;
    }
    pthread_mutex_unlock(&sem_instruccion);
    log_debug(logger_worker, "(ejecutarCreate) - File:Tag enviados");
    return false;
}


bool ejecutarTruncate(char *file, char *tag, int tamanio){

    log_debug(logger_worker, "entre a (ejecutarTruncate)");
    if (tamanio % tam_pag != 0 || tam_pag % tamanio != 0)
    {
        log_error(logger_worker, "El tamanio a truncar debe ser multiplo del tamanio de pagina");
        return true;
    }
     log_debug(logger_worker, "(ejecutarTruncate) - pase checkeo tam pag ");
    char *mensaje_truncado = string_from_format("TRUNCATE %d %s %s %d", query->id_query,file, tag, tamanio);

    Mensaje* mensajito = crearMensajito(mensaje_truncado);
     log_debug(logger_worker, "(ejecutarTruncate) - mensajito: %s", mensajito->mensaje);
     
    enviarMensajito(mensajito, socket_storage ,logger_worker);
    
    Mensaje* resp_truncate = recibirMensajito(socket_storage,logger_worker);
    if (!string_equals_ignore_case(resp_truncate->mensaje,"OK")){
        log_debug(logger_worker, "(ejecutarTruncate) Query %d finalizada", query->id_query);
        char* formato_error_master = string_from_format("ERROR %s", resp_truncate->mensaje);
        Mensaje* mensaje_error_master = crearMensajito(formato_error_master);
        enviarMensajito(mensaje_error_master, socket_master ,logger_worker);
        
        free(mensaje_truncado);
        free(formato_error_master);

        liberarMensajito(resp_truncate);
        return true;
    }
    free(mensaje_truncado);
    log_debug(logger_worker, "(ejecutarTruncate) - File:Tag y tamanios enviados");
    return false;
}

bool ejecutarWrite(char *file, char *tag, int dir_base, char *contenido_a_escribir){
    log_debug(logger_worker, "(ejecutarWrite) - Dir base: %d", dir_base);
    int nro_pagina = dir_base / tam_pag; // nro de bloque logico
    int desplazamiento = dir_base % tam_pag;
    if(!escribirEnMemoria(file, tag, nro_pagina, desplazamiento,contenido_a_escribir)){
        char* formato_error_master = string_from_format("ERROR %s", error_en_operacion);
        error_en_operacion = "OK";
        Mensaje* mensaje_error_master = crearMensajito(formato_error_master);
        enviarMensajito(mensaje_error_master, socket_master ,logger_worker);

        free(formato_error_master);
        return true;
    }
    return false;

}

bool ejecutarRead(char *file, char *tag, int dir_base, int tamanio){
    log_debug(logger_worker, "(ejecutarRead) - Dir base: %d", dir_base);
    if (tamanio % tam_pag == 0 || tam_pag % tamanio == 0)
    {
        log_error(logger_worker, "El tamanio a leer debe ser multiplo del tamanio de pagina");
        return true;
    }
    
    int pagina = dir_base / tam_pag; // capaz no toma el tam_pag global dentro de helper-worker.h
    int desplazamiento = dir_base % tam_pag;

    char *datoLeido = leerEnMemoria(file, tag, pagina, desplazamiento, tamanio);
    if(datoLeido == NULL){
        log_debug(logger_worker, "(ejecutarRead) Query %d finalizada", query->id_query);    
         char* formato_error_master = string_from_format("ERROR %s", error_en_operacion);
        error_en_operacion = "OK";
        Mensaje* mensaje_error_master = crearMensajito(formato_error_master);
        
        enviarMensajito(mensaje_error_master, socket_master ,logger_worker);
        free(formato_error_master);

        return true;
    }
    
    char* mensaje_a_master_formateado = string_from_format("READ %s %s %s",file, tag, datoLeido); // READ ID_QUERY FILE TAG CONTENIDO
    Mensaje* mensajito = crearMensajito(mensaje_a_master_formateado); 
   
    enviarMensajito(mensajito,socket_master,logger_worker); // ENVIO READ A MASTER

     free(mensaje_a_master_formateado);
    
    log_debug(logger_worker, "File:Tag y tamanios enviados");
    return false;
}

bool ejecutarTag(char *file_origen, char *tag_origen, char *file_destino, char *tag_destino){
    log_debug(logger_worker, "(ejecutarTag) - File origen: %s Tag origen: %s", file_origen, tag_origen);
    char *file_a_taggear = string_from_format("TAG %d %s %s %s %s",query->id_query, file_origen, tag_origen, file_destino, tag_destino);

    pthread_mutex_lock(&sem_instruccion);
    Mensaje* mensajito = crearMensajito(file_a_taggear);

    enviarMensajito(mensajito,socket_storage,logger_worker);
    free(file_a_taggear);
    pthread_mutex_unlock(&sem_instruccion);

     Mensaje* resp_tag = recibirMensajito(socket_storage,logger_worker);
    if (!string_equals_ignore_case(resp_tag->mensaje,"OK")){
        log_debug(logger_worker, "(ejecutarTag) Query %d finalizada", query->id_query);
        char* formato_error_master = string_from_format("ERROR %s", resp_tag->mensaje);
        Mensaje* mensaje_error_master = crearMensajito(formato_error_master);
        enviarMensajito(mensaje_error_master, socket_master ,logger_worker);
        free(formato_error_master);
        liberarMensajito(resp_tag);
        return true;
    }



    log_debug(logger_worker, "%s:%s y %s:%s destino enviados", file_origen, tag_origen, file_destino, tag_destino);
    return false;
}

bool ejecutarCommit(char *file, char *tag){
    ejecutarFlush(file, tag);
    log_debug(logger_worker, "(ejecutarCommit) - Flush realizado antes de commit");
    char *fileACommit = string_from_format("COMMIT %d %s %s",query->id_query, file, tag);

    pthread_mutex_lock(&sem_instruccion);
    Mensaje* mensajito = crearMensajito(fileACommit);
   
    enviarMensajito(mensajito,socket_storage,logger_worker);
     free(fileACommit);
    pthread_mutex_unlock(&sem_instruccion);

     Mensaje* resp_commit = recibirMensajito(socket_storage,logger_worker);
    if (!string_equals_ignore_case(resp_commit->mensaje,"OK")){
        log_debug(logger_worker, "(ejecutarCommit) Query %d finalizada", query->id_query);

        char* formato_error_master = string_from_format("ERROR %s", resp_commit->mensaje);
        Mensaje* mensaje_error_master = crearMensajito(formato_error_master);
        enviarMensajito(mensaje_error_master, socket_master ,logger_worker);
        free(formato_error_master);
        liberarMensajito(resp_commit);
        return true;
    }
    log_debug(logger_worker, "%s:%s realizar commit enviado a storage", file, tag);
    return false;
}

bool ejecutarFlush(char *file, char *tag){
    /*escribir todas las pags con bit_m = 1*/
    log_debug(logger_worker, "(ejecutarFlush) - Iniciando flush de %s:%s", file, tag);
    TablaPaginas* tabla_a_flush = buscarTablaPags(file,tag);

    t_list* entradas_obtenidas = obtenerEntradasAFlushear(tabla_a_flush);
    
    if(entradas_obtenidas == NULL){
        log_debug(logger_worker,"(ejecutarFlush) - No se encontraron entradas para flushear");
        return false; // no consulte con storage respondo ok y sigo proxima instruccion
    }

    for (int i = 0; i < list_size(entradas_obtenidas); i++){
        log_debug(logger_worker, "(ejecutarFlush) - Flusheando entrada %d de %d", i+1, list_size(entradas_obtenidas));
        EntradaDeTabla* entrada_a_persistir = list_remove(entradas_obtenidas,0);
        if(!escribirEnStorage(entrada_a_persistir)){
            char* formato_error_master = string_from_format("ERROR %s", error_en_operacion);
            error_en_operacion = "OK";
            Mensaje* mensaje_error_master = crearMensajito(formato_error_master);
            
            enviarMensajito(mensaje_error_master, socket_master ,logger_worker);

            free(formato_error_master);
            return true;
        }
    }

    list_destroy(entradas_obtenidas);

    log_debug(logger_worker, "%s:%s hacer flush enviado a storage ", file, tag);
    return false;
}

bool ejecutarDelete(char *file, char *tag){ // las páginas se van a ir limpiando a medida que corran los reemplazos.
    log_debug(logger_worker, "(ejecutarDelete) - Iniciando delete de %s:%s", file, tag);
    char* mensaje_delete = string_from_format("DELETE %d %s %s",query->id_query, file ,tag);
    Mensaje* mensajito = crearMensajito(mensaje_delete);
    
    enviarMensajito(mensajito,socket_storage,logger_worker);

    free(mensaje_delete);
    Mensaje* respuesta_delete = recibirMensajito(socket_storage,logger_worker); // ENumERrrorCapaz OK 
    if (!string_equals_ignore_case(respuesta_delete->mensaje,"OK")){
        log_debug(logger_worker, "(ejecutarDelete) Query %d finalizada", query->id_query);
         char* formato_error_master = string_from_format("ERROR %s", respuesta_delete->mensaje);
        Mensaje* mensaje_error_master = crearMensajito(formato_error_master);
        enviarMensajito(mensaje_error_master, socket_master ,logger_worker);

        free(formato_error_master);
        return true;
    }
    
    TablaPaginas* tabla_a_limpiar = buscarTablaPags(file,tag);
    list_destroy_and_destroy_elements(tabla_a_limpiar->entradas,free);
    liberarTablaPaginas(tabla_a_limpiar);     
    log_debug(logger_worker, "(ejecutarDelete) - Tabla de paginas eliminada de memoria");    
    return false;
}

bool ejecutarEnd(){
    interrumpir_query = true;

    log_debug(logger_worker, "Query %d finalizada", query->id_query);

   
    Mensaje* mensajito = crearMensajito("END");
    enviarMensajito(mensajito,socket_master,logger_worker);

    return true;

}


#include "ciclo_instruccion.h"
#include <stdint.h>

t_instruccion *instruccion;
int socket_master;
int socket_storage;
char* NOMBRE_OPERACIONES[9] = 
    {"CREATE","TRUNCATE","WRITE","READ","TAG","COMMIT","FLUSH","DELETE","END"};
int CANT_OPERACIONES_WORKER = 9;



t_list *crearListaDeInstrucciones(){
    char *path_completo = string_from_format("%s/%s",config_worker->path_queries, query->nombre);
    //------------------------------ /home/utnso/queries/query1
    //revisar si más de un worker abre el mismo archivo
    
    FILE *file_query = fopen(path_completo, "r");
    if (!file_query){
        log_error(logger_worker, "Error al abrir el archivo en la ruta %s",path_completo);
        return NULL;
    }

    fscanf(file_query, "%*s");

    t_list *lista_instucciones = list_create();
    if (!lista_instucciones)
    {
        fclose(file_query);
        return 1;
    }

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
        if (!linea)
            break;

        list_add(lineas, linea);
    }

    free(buffer);
    free(path_completo);
    fclose(file_query);

    log_info(logger_worker, "Cantidad de lineas en el archivo: %d", list_size(lista_instucciones));

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
        ejecutarCreate(file, tag);
        nombre_instruccion = "CREATE";
        // tipo_instruccion = "REQUIERO_DESALOJO";
        break;

    case TRUNCATE:
        //<NOMBRE_FILE>:<TAG> <TAMAÑO>
        int tamanio_truncate = atoi(strtok(instruccion->operacion, " "));
        ejecutarTruncate(file, tag, tamanio_truncate);
        nombre_instruccion = "TRUNCATE";
        // tipo_instruccion = "REQUIERO_DESALOJO";
        break;

    case WRITE:
        //<FILE>:<TAG> <DIRECCIÓN BASE> <CONTENIDO>
        int dir_base_write = atoi(strtok(NULL, " "));
        char *contenido = strtok(NULL, " ");

        ejecutarWrite(file, tag, dir_base_write, contenido);
        nombre_instruccion = "WRITE";
        break;

    case READ:
        //<NOMBRE_FILE>:<TAG> <DIRECCIÓN BASE> <TAMAÑO>
        int dir_base_read = atoi(strtok(NULL, " "));
        int tamanio_read = atoi(strtok(NULL, " "));

        ejecutarRead(file, tag, dir_base_read, tamanio_read);
        nombre_instruccion = "READ";
        break;

    case TAG:
        //<NOMBRE_FILE_ORIGEN>:<TAG_ORIGEN> <NOMBRE_FILE_DESTINO>:<TAG_DESTINO>
        char *file_destino = strtok(NULL, ":");
        char *tag_destino = strtok(NULL, " ");

        ejecutarTag(file, tag, file_destino, tag_destino);
        nombre_instruccion = "TAG";
        break;

    case COMMIT:
        //<NOMBRE_FILE>:<TAG>

        ejecutarCommit(file, tag);
        nombre_instruccion = "COMMIT";
        break;

    case FLUSH:
        //<NOMBRE_FILE>:<TAG>
        ejecutarFlush(file, tag);
        nombre_instruccion = "FLUSH";
        break;

    case DELETE:
        //<NOMBRE_FILE>:<TAG>
        ejecutarDelete(file, tag);
        nombre_instruccion = "DELETE";
        break;

    case END:
        ejecutarEnd();
        nombre_instruccion = "END";
        break;

    default:
        log_error(logger_worker,"Error - (Execute) - ingrese una instruccion valida");
        break;
    }

    log_info(logger_worker,"## Query %d: - Instrucción realizada: %s", query->id_query, nombre_instruccion);
    
    return finaliza;
}

void ejecutarCreate(char *file, char *tag){
    // faltaria crear alguna funcion para confirmar que se haya creado el tag para ese file (maybe)
    char *mensaje_creacion = string_from_format("CREATE %d %s %s 0");
    
    pthread_mutex_lock(&sem_instruccion);
    Mensaje* mensajito = crearMensajito(mensaje_creacion);
    
    enviarMensajito(mensajito,socket_storage,logger_worker);
    Mensaje* resp_create = recibirMensajito(socket_storage,logger_worker);
    if (!string_equals_ignore_case(resp_create->mensaje,"OK")){
        //ejecutarEnd; o algo asi
        liberarMensajito(resp_create);
        return;
    }
    pthread_mutex_unlock(&sem_instruccion);
    log_debug(logger_worker, "(ejecutarCreate) - File:Tag enviados");
}

void ejecutarTruncate(char *file, char *tag, int tamanio){

    if (tamanio % tam_pag != 0 || tam_pag % tamanio != 0)
    {
        log_error(logger_worker, "El tamanio a truncar debe ser multiplo del tamanio de pagina");
        return;
    }
    char *mensaje_truncado = string_from_format("TRUNCATE %d %s %s %d", query->id_query,file, tag, tamanio);

    Mensaje* mensajito = crearMensajito(mensaje_truncado); 
    free(mensaje_truncado); 
    enviarMensajito(mensajito,socket_storage,logger_worker);
    Mensaje* resp_truncate = recibirMensajito(socket_storage,logger_worker);
    if (!string_equals_ignore_case(resp_create->mensaje,"OK")){
        //ejecutarEnd; o algo asi
        liberarMensajito(resp_truncate);
        return;
    }
    log_debug(logger_worker, "(ejecutarTruncate) - File:Tag y tamanios enviados");
}

void ejecutarWrite(char *file, char *tag, int dir_base, char *contenido_a_escribir){
    
    int nro_pagina = dir_base / tam_pag; // nro de bloque logico
    int desplazamiento = dir_base % tam_pag;
    escribirEnMemoria(file, tag, nro_pagina, desplazamiento,contenido_a_escribir);
    
}

void ejecutarRead(char *file, char *tag, int dir_base, int tamanio){
    
    if (tamanio % tam_pag == 0 || tam_pag % tamanio == 0)
    {
        log_error(logger_worker, "El tamanio a leer debe ser multiplo del tamanio de pagina");
        return;
    }
    
    int pagina = dir_base / tam_pag; // capaz no toma el tam_pag global dentro de helper-worker.h
    int desplazamiento = dir_base % tam_pag;

    char *datoLeido = leerEnMemoria(file, tag, pagina, desplazamiento, tamanio);
    
    
    Mensaje* mensajito = crearMensajito(datoLeido);
    enviarMensajito(mensajito,socket_master,logger_worker);
    

    log_debug(logger_worker, "File:Tag y tamanios enviados");
}

void ejecutarTag(char *file_origen, char *tag_origen, char *file_destino, char *tag_destino){

    char *fileATaggear;
    sprintf(fileATaggear, "TAG %d %s %s %s %s",query->id_query, file_origen, tag_origen, file_destino, tag_destino);

    pthread_mutex_lock(&sem_instruccion);
    Mensaje* mensajito = crearMensajito(fileATaggear);
    enviarMensajito(mensajito,socket_storage,logger_worker);
    pthread_mutex_unlock(&sem_instruccion);
    log_debug(logger_worker, "%s:%s y %s:%s destino enviados", file_origen, tag_origen, file_destino, tag_destino);
}

void ejecutarCommit(char *file, char *tag){
    ejecutarFlush(file, tag);

    char *fileACommit;
    sprintf(fileACommit, "COMMIT %d %s %s",query->id_query, file, tag);

    pthread_mutex_lock(&sem_instruccion);
    Mensaje* mensajito = crearMensajito(fileACommit);
    enviarMensajito(mensajito,socket_storage,logger_worker);
    pthread_mutex_unlock(&sem_instruccion);
    log_debug(logger_worker, "%s:%s realizar commit enviado a storage", file, tag);
}

void ejecutarFlush(char *file, char *tag){
    /*escribir todas las pags con bit_m = 1*/
    TablaPaginas* tabla_a_flush = buscarTablaPags(file,tag);

    t_list* entradas_obtenidas = obtenerEntradasAFlushear(tabla_a_flush);
    
    if(entradas_obtenidas == NULL){
        log_debug(logger_worker,"(ejecutarFlush) - No se encontraron entradas para flushear");
        return;
    }

    for (int i = 0; i < list_size(entradas_obtenidas); i++){
        EntradaDeTabla* entrada_a_persistir = list_remove(entradas_obtenidas,0);
        escribirEnStorage(entrada_a_persistir);
    }

    list_destroy(entradas_obtenidas);

    log_debug(logger_worker, "%s:%s hacer flush enviado a storage ", file, tag);
}

void ejecutarDelete(char *file, char *tag){ // las páginas se van a ir limpiando a medida que corran los reemplazos.
    
    char* mensaje_delete = string_from_format("DELETE %d %s %s",query->id_query, file ,tag);
    Mensaje* mensajito = crearMensajito(mensaje_delete);
    free(mensaje_delete);
    enviarMensajito(mensajito,socket_storage,logger_worker);
    Mensaje* respuesta_delete = recibirMensajito(socket_storage,logger_worker); // ENumERrrorCapaz OK 
    if (!string_equals_ignore_case(respuesta_delete->mensaje,"OK")){
        log_error(logger_worker,"(ejecutarDelete) - Worker no pudo eliminar ");
        exit(EXIT_FAILURE);   
        // ejecutarEnd(); capaz
    }
    
    TablaPaginas* tabla_a_limpiar = buscarTablaPags(file,tag);
    list_destroy_and_destroy_elements(tabla_a_limpiar->entradas,free);
    liberarTablaPaginas(tabla_a_limpiar);         
}

void ejecutarEnd(){
    interrumpir_query = false;

    log_debug(logger_worker, "Query %d finalizada", query->id_query);

   
    Mensaje* mensajito = crearMensajito("END");
    enviarMensajito(mensajito,socket_master,logger_worker);

}

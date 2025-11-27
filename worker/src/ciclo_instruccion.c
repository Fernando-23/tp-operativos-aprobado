
#include "ciclo_instruccion.h"
#include "memoria_interna.h"
#include <stdint.h>

t_instruccion *instruccion;
int socket_master;
int socket_storage;
char* NOMBRE_OPERACIONES[9] = 
    {"CREATE","TRUNCATE","WRITE","READ","TAG","COMMIT","FLUSH","DELETE","END"};
int CANT_OPERACIONES_WORKER = 9;



t_list *crearListaDeInstrucciones(){
    char *path_completo;
    //------------------------------ /home/utnso/queries/query1
    sprintf(path_completo, "%s/%s", config_worker->path_queries, query->nombre);
    //revisar si más de un worker abre el mismo archivo
    
    FILE *file_query = fopen(path_completo, "r");
    if (!file_query){
        log_error(logger_worker, "Error al abrir el archivo en la ruta %s",path_completo);
        abort();
    }

    fscanf(file_query, "%*s");

    t_list *lineas = list_create();
    if (!lineas)
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
    fclose(file_query);

    log_info(logger_worker, "Cantidad de lineas en el archivo: %d", list_size(lineas));

    return lineas;
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

    log_info("## Query %d: FETCH - Program Counter: %d - %s", query->id_query, query->pc_query, op_code);

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
    char* file, tag;
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
        int tamanio = atoi(strtok(instruccion->operacion, " "));
        ejecutarTruncate(file, tag, tamanio);
        nombre_instruccion = "TRUNCATE";
        // tipo_instruccion = "REQUIERO_DESALOJO";
        break;

    case WRITE:
        //<FILE>:<TAG> <DIRECCIÓN BASE> <CONTENIDO>
        int dir_base = atoi(strtok(NULL, " "));
        char *contenido = strtok(NULL, " ");

        ejecutarWrite(file, tag, dir_base, contenido);
        nombre_instruccion = "WRITE";
        break;

    case READ:
        //<NOMBRE_FILE>:<TAG> <DIRECCIÓN BASE> <TAMAÑO>
        int dir_base = atoi(strtok(NULL, " "));
        int tamanio = atoi(strtok(NULL, " "));

        ejecutarRead(file, tag, dir_base, tamanio);
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
        log_Error("Error - (Execute) - ingrese una instruccion valida");
        break;
    }

    log_info("## Query %d: - Instrucción realizada: %s", query->id_query, nombre_instruccion);
    
    return finaliza;
}

void ejecutarCreate(char *file, char *tag)
{
    // faltaria crear alguna funcion para confirmar que se haya creado el tag para ese file (maybe)
    char *fileAcrear;
    sprintf(fileAcrear, "CREATE %d %s %s %d",query->id_query, file, tag, 0);
    pthread_mutex_lock(&sem_instruccion);
    Mensaje* mensajito = crearMensajito(fileAcrear);
    enviarMensajito(mensajito,socket_storage,logger_worker);
    pthread_mutex_unlock(&sem_instruccion);
    log_debug(logger_worker, "File:Tag enviados");
}

void ejecutarTruncate(char *file, char *tag, int tamanio)
{

    if (tamanio % tam_pag != 0 || tam_pag % tamanio != 0)
    {
        log_error(logger_worker, "El tamanio a truncar debe ser multiplo del tamanio de pagina");
        return;
    }
    char *fileATrunquear;
    sprintf(fileATrunquear, "TRUNCATE %d %d %s %d", query->id_query,file, tag, tamanio);
    
    pthread_mutex_lock(&sem_instruccion);
    Mensaje* mensajito = crearMensajito(fileATrunquear);  
    enviarMensajito(mensajito,socket_storage,logger_worker);
    pthread_mutex_unlock(&sem_instruccion);
    log_debug(logger_worker, "File:Tag y tamanios enviados");
}

// PROXIMAMENTE EN DBZ --- HACER TODO LO DEL PAGE FAULT

/*
    master             -                               worker
    query que creo que estas ejecutando            query ejecutando
    bool estoy esperando que se desaloje worker
    false                                                ok
      


*/

void ejecutarWrite(char *file, char *tag, int dir_base, char *contenido_a_escribir){
    int nro_pagina = dir_base / tam_pag; // nro de bloque logico
    int desplazamiento = dir_base % tam_pag;
    
    size_t bytes_contenido = strlen(contenido_a_escribir);
    size_t total_a_escribir = bytes_contenido + 1; 
    size_t escritos = 0;
    int pag_actual = nro_pagina;
    int desp_actual = desplazamiento;
    int frame_seleccionado;
    TablaPaginas* tabla_paginas = buscarOCrearTabla(file, tag);

    while(escritos < total_a_escribir){

        EntradaDeTabla* entrada_pag = buscarEntradaPagina(tabla_paginas, pag_actual);
        if (entrada_pag == NULL || entrada_pag->bit_presencia == 0){
            
            if(entrada_pag == NULL){ 
                entrada_pag = crearEntradaPagina(pag_actual,tabla_paginas);
                list_add(tabla_paginas,entrada_pag);    
            }

            frame_seleccionado = gestionarPAGE_FAULT(file,tag,nro_pagina);
            entrada_pag->nro_frame = frame_seleccionado;
        }

        void* base_frame = memoria + (entrada_pag->nro_frame * tam_pag);
        size_t espacioLibre = tam_pag - desp_actual;
        size_t por_copiar = total_a_escribir - escritos; 
        if (por_copiar > espacioLibre) por_copiar = espacioLibre; 

        // Copia del tramo que cae en ESTA página
        memcpy(base_frame + desp_actual, contenido_a_escribir + escritos, por_copiar);

        entrada_pag->bit_uso = 1;
        entrada_pag->bit_modificado = 1;
        entrada_pag->last_used_ms = now_ms();

       
        uint64_t dfis = dir_fisica(entrada_pag->nro_frame, desp_actual);
        
        log_info(logger_worker,
                 "Query %d: Acción: ESCRIBIR - Dirección Física: %llu - Valor: %.*s",
                 query->id_query,
                 (unsigned long long)dfis,
                 (const char*)(contenido_a_escribir + escritos));

        escritos += por_copiar;

        // ¿Siguiente página?
        if (por_copiar == espacioLibre && escritos < total_a_escribir) {
            pag_actual++;
            desp_actual = 0;
        } else {
            desp_actual += (int)por_copiar;
        }
    }
}

void ejecutarRead(char *file, char *tag, int dir_base, int tamanio){
    
    if (tamanio % tam_pag == 0 || tam_pag % tamanio == 0)
    {
        log_error(logger_worker, "El tamanio a leer debe ser multiplo del tamanio de pagina");
        return;
    }
    int pagina = dir_base / tam_pag; // capaz no toma el tam_pag global dentro de helper-worker.h
    int desplazamiento = dir_base % tam_pag;

    char *datoLeido = leer_en_memoria_paginada(file, tag, pagina, desplazamiento, tamanio);
    
    pthread_mutex_lock(&sem_instruccion);
    Mensaje* mensajito = crearMensajito(datoLeido);
    enviarMensajito(mensajito,socket_master,logger_worker);
    pthread_mutex_unlock(&sem_instruccion);

    log_debug(logger_worker, "File:Tag y tamanios enviados");
}



bool estaPagEnMemoria(char* file, char* tag, int nro_pag){

    TablaPaginas* tabla_a_consultar = buscarTablaPags(file,tag);
    if(tabla_a_consultar != NULL){
        EntradaDeTabla* entrada = buscarEntradaPorNroPag(tabla_a_consultar->entradas,nro_pag);
        if (entrada!=NULL){
            return (entrada->bit_presencia == 1);        
        }
        
        log_debug(logger_worker, "(estaPagEnMemoria) - No se encontro la entrada en memoria");
        return false;
    }
    
    log_debug(logger_worker,"(estaPagEnMemoria) - No se encontro la tabla en memoria");
    return false;
}




void ejecutarTag(char *file_origen, char *tag_origen, char *file_destino, char *tag_destino)
{

    char *fileATaggear;
    sprintf(fileATaggear, "TAG %d %s %s %s %s",query->id_query, file_origen, tag_origen, file_destino, tag_destino);

    pthread_mutex_lock(&sem_instruccion);
    Mensaje* mensajito = crearMensajito(fileATaggear);
    enviarMensajito(mensajito,socket_storage,logger_worker);
    pthread_mutex_unlock(&sem_instruccion);
    log_debug(logger_worker, "%s:%s y %s:%s destino enviados", file_origen, tag_origen, file_destino, tag_destino);
}

void ejecutarCommit(char *file, char *tag)
{
    ejecutarFlush(file, tag);

    char *fileACommit;
    sprintf(fileACommit, "COMMIT %d %s %s",query->id_query, file, tag);

    pthread_mutex_lock(&sem_instruccion);
    Mensaje* mensajito = crearMensajito(fileACommit);
    enviarMensajito(mensajito,socket_storage,logger_worker);
    pthread_mutex_unlock(&sem_instruccion);
    log_debug(logger_worker, "%s:%s realizar commit enviado a storage", file, tag);
}

void ejecutarFlush(char *file, char *tag)
{
    char *fileACommit;
    sprintf(fileACommit, "FLUSH %d %s %s",query->id_query, file, tag);

    pthread_mutex_lock(&sem_instruccion);
    Mensaje* mensajito = crearMensajito(fileACommit);
    enviarMensajito(mensajito,socket_storage,logger_worker);
    pthread_mutex_unlock(&sem_instruccion);

    log_debug(logger_worker, "%s:%s hacer flush enviado a storage ", file, tag);
}

void ejecutarDelete(char *file, char *tag)
{ // las páginas se van a ir limpiando a medida que corran los reemplazos.
    char *fileACommit;
    sprintf(fileACommit, "DELETE %d %s %s",query->id_query, file, tag);
    
    pthread_mutex_lock(&sem_instruccion);
    Mensaje* mensajito = crearMensajito(fileACommit);
    enviarMensajito(mensajito,socket_storage,logger_worker);
    pthread_mutex_unlock(&sem_instruccion);
    log_debug(logger_worker, "%s:%s enviados a Storage para el delete", file, tag);
}

void ejecutarEnd()
{
    interrumpir_query = false;

    log_debug(logger_worker, "Query %d finalizada", query->id_query);

    pthread_mutex_lock(&sem_instruccion);
    Mensaje* mensajito = crearMensajito("END");
    enviarMensajito(mensajito,socket_storage,logger_worker);
    pthread_mutex_unlock(&sem_instruccion);
}



EntradaDeTabla *buscar_o_crear_entrada_pagina(TablaPaginas *tabla, int pag_actual, char *file, char *tag)
{
    EntradaDeTabla *entrada = buscar_entrada_pagina(tabla, pag_actual);
    if (entrada == NULL){
        entrada = malloc(sizeof(EntradaDeTabla));
        if (entrada == NULL){
            log_error(logger_worker, "(buscar_o_crear_entrada_pagina) - Fallo malloc");
            return NULL;
        }
        // busco frame libre ya que estoy creando una nueva entrada
        Frame *frameLibre = buscar_frame_libre(file, tag);
        if (frameLibre == NULL){
            log_error(logger_worker, "No se pudo obtener un frame libre para la nueva entrada de pagina");
            free(entrada);
            return NULL;
        }

        uint64_t t = now_ms();

        entrada->nro_pag = pag_actual;
        entrada->nro_frame = frameLibre->nro_frame;
        entrada->bit_presencia = 1;
        entrada->bit_modificado = 0;
        entrada->bit_uso = 1;
        entrada->tabla = tabla;

        entrada->last_used_ms = t;

        frameLibre->entrada = entrada;  //chauuu
        bitMap[frameLibre->nro_frame] = 1;  //chau dos

        list_add(tabla->entradas, entrada);

        frameLibre->entrada = entrada; 
    }
    else{
        Frame *frameLibre = buscarFrameLibre(file, tag);
        entrada->bit_presencia = 1;
    }
    return entrada;
}
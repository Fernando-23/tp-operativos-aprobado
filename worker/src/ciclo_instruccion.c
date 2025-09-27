#include "ciclo_instruccion.h"
#include "memoria_interna.h"

t_instruccion* instruccion;
int socket_master;
int socket_storage;

t_list *crear_lista(){
    char* path_completo;
    sprintf(path_completo,"%s/%s",config_worker->path_queries,query->path_query);
    FILE *file_query = fopen(path_completo,"r");
    if (!file_query)
    {
        log_error(logger_worker,"Error al abrir el archivo");
    }
    fscanf(file_query,"%*s"); 

    t_list *lineas = list_create(); 
    if (!lineas) {
        fclose(file_query);
        return 1;
    }

    char *buffer = NULL;
    size_t tam = 0;
    ssize_t leidos;

    while ((leidos = getline(&buffer, &tam, file_query)) != -1) {
        // Sacar el salto de línea si lo tiene
        if (leidos > 0 && buffer[leidos - 1] == '\n')
            buffer[leidos - 1] = '\0';

        // strdup para copiar, porque getline reutiliza el buffer
        char *linea = strdup(buffer);
        if (!linea) break;

        list_add(lineas, linea);
    }

    free(buffer);
    fclose(file_query);


    log_info(logger_worker,"Cantidad de lineas en el archivo: %d",list_size(lineas));   
    
    
    return lineas;
}

void destruir(void *element) {free(element);}

char* Fetch(t_list* lista_de_instrucciones) {
    
    char* instruccion = list_get(lista_de_instrucciones, query->pc_query);
    
    char** instruccion_separada = string_n_split(instruccion,1," ");
    char* op_code = instruccion_separada[0];
    
    
    log_info("## Query %d: FETCH - Program Counter: %d - %s",query->id_query,query->pc_query,op_code);
    
    return instruccion;
    
}

void Decode(char* instruccionCom) {
    
    char** instruccion_separada = string_n_split(instruccionCom,1," ");
    // [cod-op,cola]

    char* op_code = instruccion_separada[0];
    instruccion->operacion = instruccion_separada[1];

    if (strcmp(op_code, "CREATE") == 0) instruccion->cod_op = CREATE;
    else if (strcmp(op_code, "TRUNCATE") == 0) instruccion->cod_op = TRUNCATE;
    else if (strcmp(op_code, "READ") == 0) instruccion->cod_op = READ;
    else if (strcmp(op_code, "WRITE") == 0) instruccion->cod_op = WRITE;
    else if (strcmp(op_code, "TAG") == 0) instruccion->cod_op = TAG;
    else if (strcmp(op_code, "COMMIT") == 0) instruccion->cod_op = COMMIT;
    else if (strcmp(op_code, "FLUSH") == 0) instruccion->cod_op = FLUSH;
    else if (strcmp(op_code, "DELETE") == 0) instruccion->cod_op = DELETE;
    else if (strcmp(op_code, "END") == 0) instruccion->cod_op = END;
}

bool Execute(){
    
    char* nombre_instruccion;
    char* file, tag;
    if ( instruccion->cod_op != END ){
        
        file = strtok(instruccion->operacion,":");
        tag = strtok(NULL," ");
    }
    
    switch (instruccion->cod_op) {

	    case CREATE:
		    //<NOMBRE_FILE>:<TAG>
            ejecutar_create(file, tag);
            nombre_instruccion = "CREATE";
		    //tipo_instruccion = "REQUIERO_DESALOJO";
            break;

	    case TRUNCATE:
            //<NOMBRE_FILE>:<TAG> <TAMAÑO>
            int tamanio = atoi(strtok(instruccion->operacion," "));
            ejecutar_truncate(file, tag, tamanio);
            nombre_instruccion = "TRUNCATE";
		    //tipo_instruccion = "REQUIERO_DESALOJO";
            break;

        case WRITE:
            //<FILE>:<TAG> <DIRECCIÓN BASE> <CONTENIDO>
            int dir_base = atoi(strtok(NULL," "));
            char* contenido = strtok(NULL," ");
            
            ejecutar_write(file,tag,dir_base,contenido);
            nombre_instruccion = "WRITE";
            break;

        case READ:
            //<NOMBRE_FILE>:<TAG> <DIRECCIÓN BASE> <TAMAÑO>
            int dir_base = atoi(strtok(NULL," "));
            int tamanio = atoi(strtok(NULL," "));

            ejecutar_read(file,tag,dir_base,tamanio);
            nombre_instruccion = "READ";
            break;
        
	
	    case TAG:
            //<NOMBRE_FILE_ORIGEN>:<TAG_ORIGEN> <NOMBRE_FILE_DESTINO>:<TAG_DESTINO>
            char* file_destino = strtok(NULL,":");
            char* tag_destino = strtok(NULL," ");
            
            ejecutar_tag(file,tag,file_destino,tag_destino);
            nombre_instruccion = "TAG";
            break;

        case COMMIT:
            //<NOMBRE_FILE>:<TAG>
            
            ejecutar_commit(file,tag);
            nombre_instruccion = "COMMIT";
                break;
    
        case FLUSH:
            //<NOMBRE_FILE>:<TAG>
            ejecutar_flush(file,tag);
            nombre_instruccion = "FLUSH";
            break;
    
        case DELETE:
            //<NOMBRE_FILE>:<TAG>
            ejecutar_delete(file,tag);
            nombre_instruccion = "DELETE";
            break;

        case END:
            ejecutar_end();
            nombre_instruccion = "END";
            break;

	    default:

		    log_Error("Error - (Execute) - ingrese una instruccion valida");
            break;
        
    }

    log_info("## Query %d: - Instrucción realizada: %s",query->id_query,nombre_instruccion);

}

void ejecutar_write(char* file, char* tag, int dir_base, char* contenido){
    int pagina = dir_base/tam_pag; // capaz no toma el tam_pag global dentro de helper-worker.h
    int desplazamiento = dir_base%tam_pag;

    int  estado = escribir_en_memoria_paginada(file,tag,pagina, desplazamiento, contenido);
    if(estado==0){
        log_info(logger_worker,"Escritura en memoria realizada con exito");
    }else{
        log_error(logger_worker,"Error no se escribio en memoria lcdm");
    }
       
}

// Devuelve 0 si OK, -1 si error
int escribir_en_memoria_paginada(char* file, char* tag, int pagina, int desplazamiento, const char* contenido)
{
    if (contenido == NULL) return -1;
    
    if (desplazamiento < 0 || desplazamiento >= tam_pag) {
        log_error(logger_worker, "Desplazamiento fuera de rango");
        return -1;
    }

    size_t bytesTotal = strlen(contenido);   // sin contar el '\0'
    size_t bytesEscritos = 0;                 // bytes de contenido ya escritos
    int pag_actual = pagina;
    int desp_actual = desplazamiento;

    tabla_paginas_t* tabla = buscar_o_crear_tabla(file, tag);

    // escribe contenido y /0
    while (1) {
        // buscar o crear y asignar frame a la entrada
        entrada_pagina_t* ent = buscar_o_crear_entrada_pagina(tabla, pag_actual);

        char* base_frame = (char*)memoria + ((size_t)ent->nro_frame * (size_t)tam_pag);
        size_t espacioLibre = (size_t)tam_pag - (size_t)desp_actual; //espacio libre

        if (bytesEscritos < bytesTotal) {
            // Aún faltan bytes del contenido por escribir
            size_t por_copiar = bytesTotal - bytesEscritos;
            if (por_copiar > espacioLibre) por_copiar = espacioLibre;

            memcpy(base_frame + desp_actual, contenido + bytesEscritos, por_copiar);
            bytesEscritos += por_copiar;
            ent->bitModificado = 1;
            ent->bitUso = 1;

            // Si llenamos la página, pasamos a la siguiente
            if (por_copiar == espacioLibre) {
                pag_actual++;
                desp_actual = 0;
                continue;
            } else {
                // Todavía queda espacio en esta página: intentaremos escribir el '\0' aquí abajo ???????????????
            }
        }
        
        // Ahora escribimos el '\0'.
        if (espacioLibre == 0) {
            // No hay lugar para el '\0' en esta página: va en la siguiente
            pag_actual++;
            desp_actual = 0;
            continue;
        } else {
            // Hay lugar para el '\0' en esta página
            base_frame[desp_actual] = '\0';
            ent->bitModificado = 1;
            ent->bitUso = 1;
            log_info(logger_worker, "Escritura completada: inicio pag=%d, fin pag=%d, bytes=%zu", pagina, pag_actual, (size_t)(bytesTotal + 1));
            return 0;
        }
    }
}

entrada_pagina_t* buscar_o_crear_entrada_pagina(tabla_paginas_t* tabla, int pag_actual){
    entrada_pagina_t* entrada = buscar_entrada_pagina(tabla, pag_actual);
    //no existe la entrada, creo un ENTRADA
    if (entrada == NULL) {
        entrada = malloc(sizeof(entrada_pagina_t));
        if (entrada == NULL) {
            log_error(logger_worker, "No se pudo asignar memoria para nueva entrada de pagina");
            return NULL;
        }
        //busco frame libre ya que estoy creando una nueva entrada
        frame_t* frameLibre= buscar_frame_libre();
        if (frameLibre == NULL) {
            log_error(logger_worker, "No se pudo obtener un frame libre para la nueva entrada de pagina");
            free(entrada);
            return NULL;
        }

        frameLibre->nro_pag=pag_actual; // DUDOSO

        entrada->nro_pag = pag_actual;
        entrada->nro_frame = frameLibre->nro_frame; 
        entrada->bitPresencia = 1;
        entrada->bitModificado = 0;
        entrada->bitUso = 1;

        list_add(tabla->entradas, entrada);
    }
    return entrada;
}

entrada_pagina_t* buscar_entrada_pagina(tabla_paginas_t* tabla, int pag_actual){
    for (int i = 0; i < list_size(tabla->entradas); i++) {
        entrada_pagina_t* entrada = list_get(tabla->entradas, i);
        if (entrada->nro_pag == pag_actual) {
            return entrada;
        }
    }
    return NULL; // No encontrada
}

frame_t* buscar_frame_libre(){
    for (int i = 0; i < cant_frames; i++)
    {
        if (bitMap[i] == 0) // si el frame esta libre
        {
            bitMap[i] = 1; // lo marco como ocupado
            return &lista_frames[i]; // retorno el frame libre
        }
    }
    // si no hay frames libres, aplico la politica de reemplazo dijo abi
    return aplicar_politica_reemplazo();
}

















void ejecutar_create(char* file, char* tag){
    // faltaria crear alguna funcion para confirmar que se haya creado el tag para ese file
    char* fileAcrear;
    sprintf(fileAcrear,"CREATE %s %s %d",file,tag,0);

    EnviarString(socket_storage,fileAcrear,logger_worker);

    log_debug(logger_worker,"File:Tag enviados");
}

void ejecutar_read(char* file, char* tag, int dir_base, int tamanio){
    if (tamanio%tam_pag==0 || tam_pag%tamanio==0){
        log_error(logger_worker,"El tamanio a leer debe ser multiplo del tamanio de pagina");
        return;
    }
    
    char* datoLeido = leer_en_memoria(file,tag,dir_base,tamanio);

    EnviarString(socket_storage,datoLeido,logger_worker);

    log_debug(logger_worker,"File:Tag y tamanios enviados");

}

char* leer_en_memoria(char* file,char* tag,int dir_base,int tamanio){
    int pagina = dir_base/tam_pag;
    int desplazamiento = dir_base%tam_pag;
    
    if(!tamanio){
        log_error(logger_worker,"Error no puedo leer ningun byte de memoria");
        return "error";
    }
    
    if (desplazamiento < 0 || desplazamiento >= tam_pag) {
        log_error(logger_worker, "Desplazamiento fuera de rango");
        return "error";
    }
    
    tabla_paginas_t* tabla = buscar_o_crear_tabla(file,tag);

    entrada_pagina_t* entrada = buscar_entrada_pagina(tabla,pagina);
    if(entrada == NULL){
        log_error(logger_worker,"Error, no se encontro la entrada de la tabla de paginas, la pagina");
        return "error";
    }

    if(entrada->nro_frame == -1){

    }
}

void ejecutar_truncate(char* file, char* tag, int tamanio){

    if (tamanio%tam_pag==0 || tam_pag%tamanio==0){
        log_error(logger_worker,"El tamanio a truncar debe ser multiplo del tamanio de pagina");
        return;
    }
    char* fileATrunquear;
    sprintf(fileATrunquear,"TRUNCATE %s %s %d",file,tag,tamanio);

    EnviarString(socket_storage,fileATrunquear,logger_worker);

    log_debug(logger_worker,"File:Tag y tamanios enviados");

}

void ejecutar_tag(char* file_origen, char* tag_origen, char* file_destino, char* tag_destino){

    char* fileATaggear;
    sprintf(fileATaggear,"TAG %s %s %s %s",file_origen,tag_origen,file_destino,tag_destino);

    EnviarString(socket_storage,fileATaggear,logger_worker);

    log_debug(logger_worker,"%s:%s y %s:%s destino enviados",file_origen,tag_origen,file_destino,tag_destino);

}

void ejecutar_commit(char* file, char* tag){
    ejecutar_flush(file,tag); 

    char* fileACommit;
    sprintf(fileACommit,"COMMIT %s %s",file,tag);

    EnviarString(socket_storage,fileACommit,logger_worker);

    log_debug(logger_worker,"%s:%s realizar commit enviado a storage",file,tag);
}

void ejecutar_flush(char* file, char* tag){
    char* fileACommit;
    sprintf(fileACommit,"FLUSH %s %s",file,tag);

    EnviarString(socket_storage,fileACommit,logger_worker);

    log_debug(logger_worker,"%s:%s hacer flush enviado a storage ",file,tag);

}


void ejecutar_delete(char* file, char* tag){      //las páginas se van a ir limpiando a medida que corran los reemplazos.
    char* fileACommit;
    sprintf(fileACommit,"DELETE %s %s",file,tag);

    EnviarString(socket_storage,fileACommit,logger_worker);

    log_debug(logger_worker,"%s:%s enviados a Storage para el delete",file,tag);
}

ejecutar_end(){
    interrumpir_query = false;

    log_debug(logger_worker,"Query %d finalizada",query->id_query);

    enviar_string(socket_master,"END",logger_worker);
}



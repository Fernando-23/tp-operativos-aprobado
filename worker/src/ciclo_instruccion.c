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

    tabla_paginas_t* tabla_asociada = buscar_o_crear_tabla(file,tag);
    
    // esto lo deberia hacer asignar pagina a frame
    frame_t* frame_libre = buscar_frame_libre();
    if (frame_libre == NULL) {
        log_error(logger_worker, "No se pudo obtener un frame, report jg");
        return;
    }

    //asigno la pagina al frame
    asignar_pagina_a_frame(tabla_asociada, pagina, frame_libre);
    size_t contenidoBytes = (size_t) contenido;

    if(hay_espacio_memoria(dir_base, contenidoBytes)){
         
        escribir_en_memoria(file,tag,pagina, desplazamiento, contenido);

        log_info(logger_worker,"Escritura en memoria realizada con exito");

    } else {
        
        log_error(logger_worker,"No hay espacio en memoria para realizar la escritura");
    }


    
}


void escribir_en_memoria(char* file, char* tag, int pagina, size_t desplazamiento, char* contenido){

    
}

// hacer un calloc para las entradas de la tabla de pagina
void asignar_pagina_a_frame(tabla_paginas_t* tabla,int pagina, frame_t* frame_libre){


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
    sprintf(fileAcrear,"%s %s %d",file,tag,0);

    EnviarString(socket_storage,fileAcrear,logger_worker);

    log_debug(logger_worker,"File:Tag enviados");
}

void ejecutar_truncate(char* file, char* tag, int tamanio){

    if (tamanio%tam_pag==0 || tam_pag%tamanio==0){
        log_error(logger_worker,"El tamanio a truncar debe ser multiplo del tamanio de pagina");
        return;
    }
    char* fileATrunquear;
    sprintf(fileATrunquear,"%s %s %d",file,tag,tamanio);

    EnviarString(socket_storage,fileATrunquear,logger_worker);

    log_debug(logger_worker,"File:Tag y tamanios enviados");

}





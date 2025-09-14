#include "ciclo_instruccion.h"

t_instruccion* instruccion;

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
    
    
    LoggerConFormato("## Query %d: FETCH - Program Counter: %d - %s",query->id_query,query->pc_query,op_code);
    
    return instruccion;
    
}

void Decode(char* instruccionCom) {
    
    char** instruccion_separada = string_n_split(instruccionCom,1," ");

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
    switch (instruccion->cod_op) {

	    case CREATE:
		    //creacion de un nuevo File
            ejecutar_create();
            nombre_instruccion = "CREATE";
		    //tipo_instruccion = "REQUIERO_DESALOJO";
            break;

	    case TRUNCATE:
            //solicitar a Storage la modificacion del tamanio del File
            ejecutar_truncate();
            nombre_instruccion = "TRUNCATE";
		    //tipo_instruccion = "REQUIERO_DESALOJO";
            break;

        case READ:

            ejecutar_read();
            nombre_instruccion = "READ";
            break;
            //
    
        case WRITE:

            ejecutar_write();
            nombre_instruccion = "WRITE";
            break;
	

	    case TAG:

            ejecutar_tag();
            nombre_instruccion = "TAG";
            break;

        case COMMIT:

            ejecutar_commit();
            nombre_instruccion = "COMMIT";
                break;
    
        case FLUSH:

            ejecutar_flush();
            nombre_instruccion = "FLUSH";
            break;
    
        case DELETE:

            ejecutar_delete();
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

    LoggerConFormato("## Query %d: - Instrucción realizada: %s",query->id_query,nombre_instruccion);

}
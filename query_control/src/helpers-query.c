#include "helpers-query.h"

ConfigQuery* config_query = NULL;
t_log* logger_query = NULL;
t_list* lista_ready = NULL;

void cargarConfigQuery(char* nombre_config_sin_extension){
    char* path_completo = string_from_format("../configs/%s.config", nombre_config_sin_extension); // tremendo

    t_config* config = iniciarConfig(path_completo);
    config_query = malloc(sizeof(ConfigQuery));
    
    if (config_query == NULL) {
		printf("(cargarConfigQuery) - Fallo asignacion config_query");
		exit(EXIT_FAILURE);
    }

    config_query->ip_master = string_duplicate(config_get_string_value(config, "IP_MASTER"));
    config_query->puerto_master = string_duplicate(config_get_string_value(config, "PUERTO_MASTER"));
    config_query->log_level = string_duplicate(config_get_string_value(config, "LOG_LEVEL"));
    
	printf("holamama\n");
    free(path_completo);
    config_destroy(config);
}

//HAY QUE LIBERAR EN ALGUN MOMENTO ESTE MENSAJE char
Mensaje* crearMensajeRegistroQuery(char* ruta,int prioridad){
	char *mensaje = string_new(); // 0 nombre_query 1
	string_append(&mensaje,"QUERY"); //Indicando que soy query a master
	string_append(&mensaje," "); //di vi no
	string_append(&mensaje,ruta); // nombre_query
	string_append(&mensaje," "); //divino
	string_append(&mensaje,string_itoa(prioridad)); 

	log_debug(logger_query,
		"Debug - (crearMensajeRegistroQuery) - Registro de Query a enviar: %s",mensaje);

	Mensaje* mensaje_registro = malloc(sizeof(Mensaje));
	mensaje_registro->mensaje = mensaje;
	mensaje_registro->size = string_length(mensaje);

	return mensaje_registro;
}

int gestionarOrdenMaestro(Mensaje* orden_de_mi_maestro){ //pasar logger

	log_debug(logger_query,"(gestionarOrdenMaestro) - Orden de mi maestro %s",orden_de_mi_maestro->mensaje);
	char **orden_cortada = string_split(orden_de_mi_maestro->mensaje," "); // 0 1
	RespuestaEnum cod_op = atoi(orden_cortada[0]);
	switch (cod_op){
	//case ERROR:
	case FINALIZAR: // 0 motivo 
		char* motivo = orden_cortada[1];
		log_info(logger_query,"### Query Finalizada - <%s>", motivo);
		string_array_destroy(orden_cortada);
		return 0; //terminar programa
		
		break;
	case LEER: // 1 nombreArchivo Tag Contenido
		char* nombre_archivo = orden_cortada[1];
		char* tag = orden_cortada[2];
		char* contenido = orden_cortada[3];
		log_info(logger_query,"Lectura realizada: Archivo %s:%s, contenido: %s",
		nombre_archivo,tag,contenido);
		string_array_destroy(orden_cortada);
		return 1; //siga nomas
		break;
	
	default:
		log_error(logger_query,"Me llego codigo de operación erroneo: %d",cod_op);
		log_debug(logger_query,"desicion: la mismisima morision del modulo");
		string_array_destroy(orden_cortada);
		return 0; //terminar programa
	}
}





/*

create -> error -> error motivo
end 




*/

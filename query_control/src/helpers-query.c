#include "helpers-query.h"

ConfigQuery* config_query = NULL;
t_log* logger_query = NULL;
t_list* lista_ready = NULL;

void CargarConfigQuery(char* path_config){
    char* path_completo = string_new();
    string_append(&path_completo, "../configs/");
    string_append(&path_completo, path_config);

    t_config* config = IniciarConfig(path_completo);
    config_query = malloc(sizeof(ConfigQuery));
    
    if (config_query == NULL) {
        abort();
    }

    config_query->ip_master = string_duplicate(config_get_string_value(config, "IP_MASTER"));
    config_query->puerto_master = string_duplicate(config_get_string_value(config, "PUERTO_MASTER"));
    config_query->log_level = config_get_int_value(config, "LOG_LEVEL");
    
    free(path_completo);
    config_destroy(config);
}

//HAY QUE LIBERAR EN ALGUN MOMENTO ESTE MENSAJE char
Mensaje* crearMensajeRegistroQuery(char* ruta,int prioridad){
	char *mensaje = string_new();
	string_append(&mensaje,ruta);
	string_append(&mensaje," "); //divino
	string_append(&mensaje,string_itoa(prioridad));

	printf("PRUEBA - (crearMensajeRegistroQuery) - Registro concatenado: %s\n",mensaje);

	Mensaje* mensaje_registro = malloc(sizeof(Mensaje));
	mensaje_registro->mensaje = mensaje;
	mensaje_registro->size = string_length(mensaje);

	return mensaje_registro;
}

int gestionarOrdenMaestro(Mensaje* orden_de_mi_maestro){ //pasar logger

	char **orden_cortada = string_split(orden_de_mi_maestro->mensaje," ");
	int cod_op = atoi(orden_cortada[0]);
	switch (cod_op){
	case 0:
		char* motivo = orden_cortada[1];
		printf("###Query Finalizada - <%s>\n", motivo);
		string_array_destroy(orden_cortada);
		return 1; //terminar programa
		
		break;
	case 1:
		char* nombre_archivo = orden_cortada[1];
		char* tag = orden_cortada[2];
		char* contenido = orden_cortada[3];
		printf("### Lectura realizada: Archivo <%s:%s>, contenido: <%s>\n",nombre_archivo,tag,contenido);
		string_array_destroy(orden_cortada);
		return 0; //siga nomas
		break;
	
	default:
		printf("me llego codigo de operación erroneo: %d\n",cod_op);
		printf("desicion matar modulo\n");
		string_array_destroy(orden_cortada);
		return 1; //terminar programa
	}
}

#include "helpers.h"


const int CANT_MODULOS = 4;
char* NOMBRE_MODULOS[CANT_MODULOS] = {"QUERY","MASTER","WORKER","STORAGE"};


const int CANT_ERRORES = 9;
char* NOMBRE_ERRORES[CANT_ERRORES] = 
	{"OK","FILE_INEXISTENTE", "TAG_INEXISTENTE","FILE_PREEXISTENTE","TAG_PREEXISTENTE",
	 "ESPACIO_INSUFICIENTE","ESCRITURA_NO_PERMITIDA","LECTURA_FUERA_DE_LIMITE","ESCRITURA_FUERA_DE_LIMITE"};


t_log* iniciarLogger(char* nombre_modulo,int nivel_log)
{
    t_log* nuevo_logger;
    
    char* nombre_log_final = string_new();
    string_append(&nombre_log_final, nombre_modulo);
    string_append(&nombre_log_final, ".log");

    nuevo_logger = log_create(nombre_log_final,nombre_modulo,true,nivel_log); //ultimo parametro es un int no un t_log
    if(nuevo_logger==NULL){
        abort();
    }
    return nuevo_logger;
}

t_config* iniciarConfig(char* nombre_config)
{
    t_config* nuevo_config;

    nuevo_config = config_create(nombre_config);
    if (nuevo_config == NULL) {
    
    abort();
    }
    return nuevo_config;
}

void chequearArgs(int cant_args_ingresados,int limite_cant_args){
    if (cant_args_ingresados > limite_cant_args) {
        printf("Error - (main) - Cantidad de argumentos invalida");
        abort();
    } 
}

void *serializar_paquete(t_paquete *paquete, uint32_t bytes)
{
	void *magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return magic;
}

void eliminar_paquete(t_paquete *paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

t_list *recibir_paquete(int socket_cliente){

	uint32_t size;
	int desplazamiento = 0;
	void *buffer;
	t_list *valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);

	while (desplazamiento < size){
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);
		char *valor = malloc(tamanio);
		memcpy(valor, buffer + desplazamiento, tamanio);
		desplazamiento += tamanio;
		list_add(valores, valor);
	}

	free(buffer);
	return valores;
}

Mensaje* crearMensajito(char* mensaje){
	Mensaje* mensajito_nuevo = malloc(sizeof(Mensaje));
	mensajito_nuevo->size = string_length(mensaje)+1;
	mensajito_nuevo->mensaje = string_new();
	mensajito_nuevo->mensaje = mensaje;
	return mensajito_nuevo;
}

void enviarMensajito(Mensaje* mensaje_a_enviar,int fd,t_log* logger){ //envia query
	
	send(fd,&mensaje_a_enviar->size,sizeof(int),0);
	// log_debug(logger,"Debug - (enviarMensajito) - Mensaje Length: %d\n",mensaje_a_enviar->size);
	
	send(fd,mensaje_a_enviar->mensaje,mensaje_a_enviar->size,0);
	// log_debug(logger,"Debug - (enviarMensajito) - Mensaje: %s\n",mensaje_a_enviar->mensaje);

	liberarMensajito(mensaje_a_enviar);
}

//RESERVA MEMORIA
Mensaje* recibirMensajito(int socket_cliente){

	Mensaje* mensajito = malloc(sizeof(Mensaje));
    recv(socket_cliente, &(mensajito->size), sizeof(int), 0);
    printf("PRUEBAS - (RecibirMensajito) - Recibi el tamanio: %d\n",mensajito->size);
   
    mensajito->mensaje = malloc(mensajito->size);
    
    recv(socket_cliente, mensajito->mensaje, mensajito->size, 0);
    printf("PRUEBAS - (RecibirMensajito) - Recibi el mensaje: %s\n",mensajito->mensaje);

	return mensajito;
}

void liberarMensajito(Mensaje* mensajito_a_liberar){
	free(mensajito_a_liberar->mensaje);
	free(mensajito_a_liberar);
}

int obtenerModuloCodOp(char *string_modulo){
    for (int i = 0; i < CANT_MODULOS; i++)
    {
        if (strcmp(NOMBRE_MODULOS[i],string_modulo)==0)
		 return i;
    }
    return -1;
}

Mensaje* mensajitoOk(){
    Mensaje* mensajito = malloc(sizeof(Mensaje));
    mensajito->mensaje = string_new();
    mensajito->mensaje = "OK";
    mensajito->size = string_length(mensajito->mensaje);
    return mensajito;
}

Mensaje* mensajitoError(ErrorStorageEnum cod_error){
    Mensaje* mensajito = malloc(sizeof(Mensaje));
	
	char* aux_armado_msg_error; 
	//---------------------------ERROR MOTIVO_ERROR
	sprintf(aux_armado_msg_error,"ERROR %s", NOMBRE_ERRORES[cod_error]);

    mensajito->mensaje = string_duplicate(aux_armado_msg_error);
    mensajito->size = string_length(mensajito->mensaje);
	
    return mensajito;
}



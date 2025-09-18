#include "helpers.h"

char* nombre_modulos[4] = {"QUERY","MASTER","WORKER","STORAGE"};

t_log* IniciarLogger(char* nombre_modulo,int nivel_log)
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

t_config* IniciarConfig(char* nombre_config)
{
    t_config* nuevo_config;

    nuevo_config = config_create(nombre_config);
    if (nuevo_config == NULL) {
    
    abort();
    }
    return nuevo_config;
}

void ChequearArgs(int cant_args_ingresados,int limite_cant_args){
    if (cant_args_ingresados > limite_cant_args) {
        printf("Error - (main) - Cantidad de argumentos invalida");
        abort();
    } 
}

void enviar_mensaje(char *mensaje, int socket_cliente){
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	uint32_t bytes = paquete->buffer->size + 2 * sizeof(uint32_t);

	void *a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
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

void RecibirMensaje(t_mensaje* mensaje,int socket_cliente){
    
    recv(socket_cliente, &(mensaje->tamanio_msg), sizeof(int), MSG_WAITALL);
    mensaje->mensaje = malloc(mensaje->tamanio_msg);
    recv(socket_cliente, mensaje->mensaje, mensaje->tamanio_msg, MSG_WAITALL);

}

void EnviarMensaje(char* mensaje,int socket_cliente){
    
    int tamanio_msg = strlen(mensaje)+1;
    send(socket_cliente, &tamanio_msg, sizeof(int), 0);
    send(socket_cliente, mensaje, tamanio_msg, 0);

}

void enviarMensajito(Mensaje* mensaje_a_enviar,int socket_servidor){ //envia query
	
	send(socket_servidor,&mensaje_a_enviar->size,sizeof(int),0);
	printf("PRUEBAS - (enviarMensajito) - Mensaje Length: %d\n",mensaje_a_enviar->size);
	
	send(socket_servidor,mensaje_a_enviar->mensaje,mensaje_a_enviar->size,0);
	printf("PRUEBAS - (enviarMensajito) - Mensaje: %s\n",mensaje_a_enviar->mensaje);

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

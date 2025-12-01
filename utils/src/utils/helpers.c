#include "helpers.h"


char* NOMBRE_MODULOS[CANT_MODULOS] = {"QUERY","MASTER","WORKER","STORAGE"};
char* NOMBRE_ERRORES[CANT_ERRORES] = 
	{"OK","FILE_INEXISTENTE", "TAG_INEXISTENTE","FILE_PREEXISTENTE","TAG_PREEXISTENTE",
	 "ESPACIO_INSUFICIENTE","ESCRITURA_NO_PERMITIDA","LECTURA_FUERA_DE_LIMITE","ESCRITURA_FUERA_DE_LIMITE"};

char* NOMBRE_RESPUESTA_WORKER[5] = {"LEER","DESALOJAR","FINALIZAR","ERROR", "FINALIZAR_WORKER"}; //LEER,

char* path_base_query = "/home/utnso/tp-2025-2c-Nombre-que-llamar-un-ayudante-/queries";


t_log* iniciarLogger(char* nombre_modulo,int nivel_log)
{
	char* nombre_archivo = string_from_format("%s.log", nombre_modulo);


    t_log* nuevo_logger = log_create(nombre_archivo, nombre_modulo, true, nivel_log); //ultimo parametro es un int no un t_log
    if(!nuevo_logger){
        fprintf(stderr, "ERROR - (iniciarLogger) - No se pudo crear el logger para %s\n", nombre_modulo);
        free(nombre_archivo);
        exit(EXIT_FAILURE);
    }

	log_info(nuevo_logger, "Logger iniciado correctamente");
	free(nombre_archivo);

    return nuevo_logger;
}

t_config* iniciarConfig(char* nombre_config)
{
    t_config* nuevo_config = config_create(nombre_config);
    if (nuevo_config == NULL) {
		fprintf(stderr,"Error - (iniciarConfig) - Ruta de config incorrecta\n");
    	exit(EXIT_FAILURE);
    }
    return nuevo_config;
}

bool chequearCantArgs(int arg, int cant_esperada){
	if (arg < cant_esperada) {
		fprintf(stderr,"Error - (chequearCantArgs) - Cantidad de argumentos invalida\n");
        return 0;
    }
	return 1;
}

void chequearCantArgsPasadosPorTerminal(int argc, int cant_esperada) {
    if (argc < cant_esperada + 1) {   // +1 por argv[0]
		fprintf(stderr,"Error - (chequearCantArgsPasadosPorTerminal) - Cantidad de argumentos invalida\n");
        exit(EXIT_FAILURE);
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
	mensajito_nuevo->size = string_length(mensaje);
	mensajito_nuevo->mensaje = string_duplicate(mensaje);
	return mensajito_nuevo;
}

void enviarMensajito(Mensaje* mensaje_a_enviar,int fd,t_log* logger){ //envia query
	
	send(fd,&mensaje_a_enviar->size,sizeof(int),0);

	send(fd,mensaje_a_enviar->mensaje,mensaje_a_enviar->size,0);
	
	log_debug(logger, "(enviarMensajito) - Mensaje enviado -> Tamaño: %d | Contenido: %s", mensaje_a_enviar->size, mensaje_a_enviar->mensaje);

	liberarMensajito(mensaje_a_enviar); // creo que deberia liberar afuera pero con cuidado queda
}

//RESERVA MEMORIA
Mensaje* recibirMensajito(int socket_cliente, t_log* logger){

	Mensaje* mensajito = malloc(sizeof(Mensaje));


    if(recv(socket_cliente, &(mensajito->size), sizeof(int), 0) <= 0){
		log_warning(logger, "(recibirMensajito) - Conexión cerrada o error en recv() del tamaño.");
        free(mensajito);
        return NULL;
	}
   
    mensajito->mensaje = malloc(mensajito->size + 1); //+1 para el \0

	if (mensajito->mensaje == NULL) {
        log_error(logger, "(recibirMensajito) - Fallo en malloc del mensaje en si mismo");
        free(mensajito);
        return NULL;
    }

    
    if(recv(socket_cliente, mensajito->mensaje, mensajito->size, 0) <= 0){
		log_error(logger, "(recibirMensajito) - Error recibiendo el mensaje en si mismo o conexión cerrada a mitad de mensaje.");
        liberarMensajito(mensajito);
		return NULL;
	}

	mensajito->mensaje[mensajito->size] = '\0'; // faltaba esto

	log_debug(logger, "(recibirMensajito) - Recibi el mensaje: Tamaño: %d", mensajito->size);
    log_debug(logger, "(recibirMensajito) - Contenido: %s", mensajito->mensaje);

	return mensajito;
}

void liberarMensajito(Mensaje* mensajito_a_liberar){
	if (mensajito_a_liberar == NULL) return; // me atajo
    if (mensajito_a_liberar->mensaje != NULL) {
        free(mensajito_a_liberar->mensaje);
    }
	free(mensajito_a_liberar);
}

int obtenerModuloCodOp(char *string_modulo){
    for (int i = 0; i < CANT_MODULOS; i++){	
        if (strcmp(NOMBRE_MODULOS[i],string_modulo)==0)
			return i;
    }
    return -1;
}

int obtenerRespuestaWorkerEnum(char* string_cod_op){
	for (int i = 0; i < 4; i++){
    	if (string_equals_ignore_case(NOMBRE_RESPUESTA_WORKER[i],string_cod_op)==0)
			return i;
    }
    return -1;
}



Mensaje* mensajitoOk(){
    Mensaje* mensajito = malloc(sizeof(Mensaje));
    mensajito->mensaje = string_duplicate("OK");
    mensajito->size = string_length(mensajito->mensaje);
    return mensajito;
}



/*
Tremendo Fer

string_from_format(formato) es como sprintf pero de las commons y reserva memoria


*/

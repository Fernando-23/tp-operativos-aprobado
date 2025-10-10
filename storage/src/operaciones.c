#include "operaciones.h"
#include "helpers-storage.h"
#include "../../utils/src/utils/helpers.h"

void* recursosHumanos(void* args_sin_formato){
    int socket_cliente = *(int *)args_sin_formato;
    
    while(1){
        int fd_cliente;
        fd_cliente = esperarCliente(socket_cliente,logger_storage);
        
        pthread_t thread_labubu;
        pthread_create(&thread_labubu, NULL, atenderLaburanteDisconforme, (void *)&fd_cliente);
        pthread_detach(thread_labubu);
    } // n veces porque hay n workers
}

void *atenderLaburanteDisconforme(void* args_sin_formato){
    int fd_cliente = *((int *)args_sin_formato);

    handshake(fd_cliente);
    
    do{
        pedidoDeLaburante(fd_cliente);
    }while(1);
}

void pedidoDeLaburante(int mail_laburante){
    
     Mensaje* mensajito;
      mensajito = recibirMensajito(mail_laburante);
      
    log_debug(logger_storage,
        "Debug - (gestionarClienteIndividual) - Recibi el mensaje %s",mensajito->mensaje);

    char** mensajito_cortado = string_split(mensajito->mensaje," ");
    CodOperacionStorage tipo_operacion = obtenerModuloCodOperacion(mensajito_cortado[0]);
    
        
    switch (tipo_operacion){
        
    case CREATE:
        enviarMensajito(mensajitoOk(), mail_laburante);
        
        string_array_destroy(mensajito_cortado);
        
        break;
    case TRUNCATE:
        enviarMensajito(mensajitoOk(), mail_laburante);
        
        string_array_destroy(mensajito_cortado);
        
        break;

    case TAG:
        enviarMensajito(mensajitoOk(), mail_laburante);
        
        string_array_destroy(mensajito_cortado);
        
        break;

    case COMMIT:
        enviarMensajito(mensajitoOk(), mail_laburante);
        
        string_array_destroy(mensajito_cortado);
        
        break;

    case FLUSH:
        enviarMensajito(mensajitoOk(), mail_laburante);
        
        string_array_destroy(mensajito_cortado);
        break;

    case DELETE:
        enviarMensajito(mensajitoOk(), mail_laburante);
        
        string_array_destroy(mensajito_cortado);
        break;
        
    case LEER_BLOQUE:
        enviarMensajito(mensajitoOk(), mail_laburante);
        
        string_array_destroy(mensajito_cortado);
        break;
    
    case ACTUALIZAR_FRAME_MODIFICADO:
        enviarMensajito(mensajitoOk(), mail_laburante);
        
        string_array_destroy(mensajito_cortado);
        break;

    case ERROR:
    
        enviarMensajito(mensajitoOk(), mail_laburante);
        
        string_array_destroy(mensajito_cortado);
        break;
    
    default:
        log_error(logger_storage,"Error - (pedidoDeLaburante) - Codigo de operacion erroneo"); //capaz un abort, quien sabe
        string_array_destroy(mensajito_cortado);
        break;
    }
    
    liberarMensajito(mensajito);

}

void handshake(int fd){
    Mensaje* mensajito_hanshake = malloc(sizeof(Mensaje));
    mensajito_hanshake->mensaje = string_itoa(datosFS->tamanio_bloque);
    mensajito_hanshake->size = string_length(mensajito_hanshake->mensaje);
    
    enviarMensajito(mensajito_hanshake,fd);
}


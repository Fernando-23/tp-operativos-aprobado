#ifndef OPERACIONES_H_
#define OPERACIONES_H_

typedef enum{
    CREATE,
    TRUNCATE,
    TAG,
    COMMIT,
    FLUSH,
    DELETE,
    LEER_BLOQUE,
    ACTUALIZAR_FRAME_MODIFICADO,
    ERROR
}CodOperacionStorage;

void* recursosHumanos(void*);//atenderClientes

void* atenderLaburanteDisconforme(void*);//atenderCliente
void pedidoDeLaburante(int mail_laburante);
void handshake(int fd);


#endif //OPERACIONES_H_
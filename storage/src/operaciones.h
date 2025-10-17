#ifndef OPERACIONES_H_
#define OPERACIONES_H_
#include "utils_storage.h"
#include <sys/stat.h>
#include "../../utils/src/utils/helpers.h"

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
void realizarCREATE(char* nombre_file,char* nombre_tag);
t_config* crearMetadata(char* path_tag);
File* crearFile(char* nombre_file,char* path_file);
Tag* crearTag(char* nombre_tag,char* path_tag);
void crearDirectorio(char* path_directorio);

#endif //OPERACIONES_H_
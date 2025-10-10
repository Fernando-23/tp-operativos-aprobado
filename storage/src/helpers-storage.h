#ifndef HELPERS_STORAGE_H_
#define HELPERS_STORAGE_H_
#include "../../utils/src/utils/conexiones.h"
#include "../../utils/src/utils/helpers.h"
#include <pthread.h>
typedef struct {
    char* puerto_escucha;
    char* fresh_start;
    char* punto_montaje;
    int retardo_operacion;
    int retardo_acceso_bloque;
    int log_level;
} ConfigStorage;

typedef struct{
    int tamanio_fsystem;
    int tamanio_bloque;
}DatosFS;

extern ConfigStorage* config_storage;
extern DatosFS* datosFS;
extern t_log* logger_storage;

void cargarConfigStorage(char* path_config);
void cargarDatosFS();


#endif // HELPERS_STORAGE_H_
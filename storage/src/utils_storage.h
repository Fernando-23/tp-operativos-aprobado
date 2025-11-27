#ifndef UTILS_STORAGE_H_
#define UTILS_STORAGE

#include <unistd.h>
#include <pthread.h>
#include <commons/string.h>
#include <commons/crypto.h>
#include "estructuras.h"





int obtenerTareaCodOperacion(char *string_modulo);

void iniciarStorage();
void inicializarSemaforos();
void cargarConfigStorage(char* path_config);
void cargarConfigSuperblock();
int calcularCantBloques();
void crearBloquesFisicos();
void crearYAgregarBloqueFisicoIndividual(int id,char* nombre_bloque);
void handshake(int fd);
void limpiarHashIndexConfig();


Mensaje* mensajitoError(ErrorStorageEnum cod_error);

#endif //UTILS_STORAGE
#ifndef UTILS_STORAGE_H_
#define UTILS_STORAGE
#include "estructuras.h"
#include <commons/string.h>
#include <unistd.h>
#include <pthread.h>



void iniciarStorage();
void cargarConfigStorage(char* path_config);
void cargarConfigSuperblock();
int calcularCantBloques();
void crearBloquesFisicos();
void crearYAgregarBloqueFisicoIndividual(int id,char* nombre_bloque);
void handshake(int fd);

#endif //UTILS_STORAGE
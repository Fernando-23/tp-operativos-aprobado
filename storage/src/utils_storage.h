#ifndef UTILS_STORAGE_H_
#define UTILS_STORAGE

#include <unistd.h>
#include <pthread.h>
#include <commons/string.h>
#include <commons/crypto.h>
#include <errno.h>
#include <sys/stat.h>
#include "estructuras.h"
#include "operaciones.h"





int obtenerTareaCodOperacion(char *string_codop);

void iniciarStorage();
void inicializarSemaforos();
void cargarConfigStorage(char* arch_config);
void cargarConfigSuperblock();
int calcularCantBloques();
void crearBloquesFisicos();
void crearYAgregarBloqueFisicoIndividual(int id,char* nombre_bloque, char* ruta_absoluta);
void handshake(int fd);
void limpiarHashIndexConfig();


Mensaje* mensajitoError(ErrorStorageEnum cod_error);

#endif //UTILS_STORAGE
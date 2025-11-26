#ifndef UTILS_STORAGE_H_
#define UTILS_STORAGE

#include <unistd.h>
#include <pthread.h>
#include <commons/string.h>
#include <commons/crypto.h>
#include "estructuras.h"


#define CANT_ERRORES 9

extern char* NOMBRE_ERRORES[CANT_ERRORES];

typedef enum {
	OK,
	FILE_INEXISTENTE,
	TAG_INEXISTENTE,
	FILE_PREEXISTENTE,
	TAG_PREEXISTENTE,
	ESPACIO_INSUFICIENTE,
	ESCRITURA_NO_PERMITIDA,
	LECTURA_FUERA_DE_LIMITE,
	ESCRITURA_FUERA_DE_LIMITE
}ErrorStorageEnum;





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
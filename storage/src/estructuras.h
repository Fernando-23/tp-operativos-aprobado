#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_
#include "../../utils/src/utils/helpers.h"
#include <commons/bitarray.h>
#include <pthread.h>

// /utnso/home/tp-blablabla/storage/
typedef struct{
    int id_fisico;
    char* nombre;
    char* ruta_absoluta;
}BloqueFisico;

// ### /storage/files/arch1/tags...
typedef struct{
    char* nombre_file; // /utnso/home/tp-blablabla/storage/files/arch1
    t_list* tags; // tipo Tag
}File;

typedef struct{
    char* nombre_tag;
    char* directorio;
    t_config* metadata_config_tag; //config del tag
    t_list* bloques_logicos; //lista de bloques logicos asignados al tag que son hardlinks a fisicos
}Tag;


typedef struct{
    int id_logico; //id del bloque logico
    char* ruta_hl; //para mas comodidad
    char* nombre;
    BloqueFisico* ptr_bloque_fisico; //puntero al bloque fisico asociado
}BloqueLogico;

typedef struct {
    char* puerto_escucha;
    char* fresh_start;
    char* punto_montaje;
    int retardo_operacion;
    int retardo_acceso_bloque;
    int log_level;
}ConfigStorage;

typedef struct{
    int tamanio_fsystem;
    int tamanio_bloque;
    int cant_bloques;
}ConfigSuperblock;

typedef struct{
    size_t tamanio;
    void* contenido;
}DatosParaHash;

typedef struct{
    bool hubo_bloques_libres;
    char** bloques_encontrados;
}RespuestaConsultaBitmap;

extern t_config* hash_index_config_gb;
extern ConfigStorage* config_storage;
extern ConfigSuperblock* datos_superblock_gb;
extern t_log* logger_storage;
extern t_list* bloques_fisicos_gb;
extern t_list* lista_files_gb;
extern char* bitmap_mmap_gb;
extern t_bitarray* bitmap_gb;
extern int cant_bloques_en_bytes_gb;
extern pthread_mutex_t mutex_bitmap;
extern pthread_mutex_t mutex_files;
extern pthread_mutex_t mutex_bloques_fisicos;
extern pthread_mutex_t mutex_cant_workers;
extern int cant_workers_conectados;
extern char* NOMBRE_CODOP_STORAGE[8];


extern char* PATH_PHYSICAL_BLOCKS;
extern char* RUTA_BASE_STORAGE;
extern char* RUTA_FILES;
extern char* RUTA_HASH_INDEX;
extern char* RUTA_AUX_FSTART_HASH_INDEX;


#endif //ESTRUCTURAS_H_
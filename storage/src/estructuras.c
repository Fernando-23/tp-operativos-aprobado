#include "estructuras.h"

ConfigStorage* config_storage = NULL;
t_config* hash_index_config_gb = NULL;
t_log* logger_storage = NULL;
t_list* bloques_fisicos_gb = NULL;
t_list* lista_files_gb = NULL;
ConfigSuperblock* datos_superblock_gb = NULL;
char* bitmap_mmap_gb = NULL;
int cant_bloques_en_bytes_gb;
int cant_workers_conectados;
t_bitarray* bitmap_gb = NULL;

pthread_mutex_t mutex_bitmap;
pthread_mutex_t mutex_files;
pthread_mutex_t mutex_bloques_fisicos;
pthread_mutex_t mutex_cant_workers;
pthread_mutex_t mutex_hash_index;


/*
char* RUTA_BITMAP = "/home/utnso/so-deploy/tp-2025-2c-Nombre-que-llamar-un-ayudante-/storage/bitmap.bin";
char* RUTA_BASE_TAG = "/home/utnso/so-deploy/tp-2025-2c-Nombre-que-llamar-un-ayudante-/storage/files/initial_file/BASE";
char* PATH_PHYSICAL_BLOCKS = "/home/utnso/so-deploy/tp-2025-2c-Nombre-que-llamar-un-ayudante-/storage/physical_blocks";
char* RUTA_BASE_STORAGE = "/home/utnso/so-deploy/tp-2025-2c-Nombre-que-llamar-un-ayudante-/storage"; //config_storage->PUNTO_MONTAJE;
char* RUTA_FILES = "/home/utnso/so-deploy/tp-2025-2c-Nombre-que-llamar-un-ayudante-/storage/files";
char* RUTA_HASH_INDEX = "/home/utnso/so-deploy/tp-2025-2c-Nombre-que-llamar-un-ayudante-/storage/configs/blocks_hash_index.config";
char* RUTA_AUX_FSTART_HASH_INDEX = "/home/utnso/so-deploy/tp-2025-2c-Nombre-que-llamar-un-ayudante-/storage/configs/aux_fstart_hash.config";
*/
char* RUTA_BITMAP = "/home/utnso/Desktop/tp-2025-2c-Nombre-que-llamar-un-ayudante-/storage/bitmap.bin";
char* RUTA_BASE_TAG = "/home/utnso/Desktop/tp-2025-2c-Nombre-que-llamar-un-ayudante-/storage/files/initial_file/BASE";
char* PATH_PHYSICAL_BLOCKS = "/home/utnso/Desktop/tp-2025-2c-Nombre-que-llamar-un-ayudante-/storage/physical_blocks";
char* RUTA_BASE_STORAGE = "/home/utnso/Desktop/tp-2025-2c-Nombre-que-llamar-un-ayudante-/storage"; //config_storage->PUNTO_MONTAJE;
char* RUTA_FILES = "/home/utnso/Desktop/tp-2025-2c-Nombre-que-llamar-un-ayudante-/storage/files";
char* RUTA_HASH_INDEX = "/home/utnso/Desktop/tp-2025-2c-Nombre-que-llamar-un-ayudante-/storage/configs/blocks_hash_index.config";
char* RUTA_AUX_FSTART_HASH_INDEX = "/home/utnso/Desktop/tp-2025-2c-Nombre-que-llamar-un-ayudante-/storage/configs/aux_fstart_hash.config";

char* NOMBRE_CODOP_STORAGE[8] = {"CREATE","TRUNCATE","TAG","COMMIT","DELETE","LEER_BLOQUE",
    "ESCRIBIR_BLOQUE","CARTA_DOCUMENTO"};
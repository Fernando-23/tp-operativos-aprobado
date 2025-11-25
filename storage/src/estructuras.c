#include "estructuras.h"

ConfigStorage* config_storage = NULL;
t_config* hash_index_config_gb = NULL;
t_log* logger_storage = NULL;
t_list* bloques_fisicos_gb = NULL;
t_list* lista_files_gb = NULL;
ConfigSuperblock* datos_superblock_gb = NULL;
char* bitmap_mmap_gb = NULL;
int cant_bloques_en_bytes_gb;
t_bitarray* bitmap_gb = NULL;


const char* PATH_PHYSICAL_BLOCKS = "/home/utnso/storage/physical_blocks";
const char* RUTA_BASE_STORAGE = "/home/utnso/storage/"; //config_storage->PUNTO_MONTAJE;
const char* RUTA_FILES = "/home/utnso/storage/files";
const char* RUTA_HASH_INDEX = "/home/utnso/storage/configs/blocks_hash_index.config";
const char* RUTA_AUX_FSTART_HASH_INDEX = "/home/utnso/storage/configs/aux_fstart_hash.config";
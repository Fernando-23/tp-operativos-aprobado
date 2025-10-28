#include "estructuras.h"

ConfigStorage* config_storage = NULL;
t_log* logger_storage = NULL;
t_list* bloques_fisicos_gb = NULL;
t_list* files_gb = NULL;
ConfigSuperblock* datos_superblock_gb = NULL;
char* bitmap_mmap_gb = NULL;
int cant_bloques_en_bytes_gb;
t_bitarray* bitmap_gb = NULL;
char* PATH_PHYSICAL_BLOCKS = "/storage/physical_blocks/";
#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_
#include "../../utils/src/utils/helpers.h"

typedef struct{
    int id_fisico;
    char* nombre;
    int contador_hard_links;
}BloqueFisico;

// ### /storage/files/archX/tags...
typedef struct{
    char* nombre_file;
    t_list* tags; // tipo Tag
}File;


typedef struct{
    char* nombre_tag;
    t_config* metadata_config_tag; //config del tag
    t_list* bloques_logicos; //lista de bloques logicos asociados al tag que son hardlinks a fisicos
    int tamanio;
}Tag;


typedef struct{
    int id_logico; //id del bloque logico
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

extern ConfigStorage* config_storage;
extern ConfigSuperblock* datos_superblock_gb;
extern t_log* logger_storage;
extern t_list* bloques_fisicos_gb;
extern t_list* files_gb;

#endif //ESTRUCTURAS_H_
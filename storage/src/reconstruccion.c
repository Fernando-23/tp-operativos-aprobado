#include "reconstruccion.h"

void RECONSTRUCCION(){

    crearBloquesFisicos();

    DIR *d_files = opendir(RUTA_FILES);
    if (!d_files){
        log_debug(logger_storage,"(RECONSTRUCCION) - no existia el directorio /files");
    } 
        
    struct dirent *dir_file;
    
    while ((dir_file = readdir(d_files)) != NULL) {
        log_debug(logger_storage,"(RECONSTRUCCION) - Leyendo File: %s",dir_file->d_name);
        if (dir_file->d_type != DT_DIR) continue;
        if (strcmp(dir_file->d_name, ".") == 0 || strcmp(dir_file->d_name, "..") == 0) continue;


        File* file = crearFile(dir_file->d_name);
        // list_add(lista_files_gb,file);
        char path_curr_file[1024];
        snprintf(path_curr_file, sizeof(path_curr_file), "%s/%s", RUTA_FILES, dir_file->d_name);
        log_debug(logger_storage,"(RECONSTRUCCION) - Path File Actual: %s",path_curr_file);
        DIR *d_tags = opendir(path_curr_file);
        if (!d_tags) continue;

        struct dirent *dir_tag;
        while ((dir_tag = readdir(d_tags)) != NULL) {
            log_debug(logger_storage,"(RECONSTRUCCION) - Leyendo Tag: %s",dir_tag->d_name);
            if (dir_tag->d_type != DT_DIR) continue;
            if (strcmp(dir_tag->d_name, ".") == 0 || strcmp(dir_tag->d_name, "..") == 0) continue;
            
            Tag* tag = crearTag(dir_tag->d_name, file->nombre_file);
            char** bloques_logs_a_reconstruir = config_get_array_value(tag->metadata_config_tag, "BLOCKS");
            for(int i = 0; i < string_array_size(bloques_logs_a_reconstruir); i++){
                int id_bloque_fisico = atoi(bloques_logs_a_reconstruir[i]);
                BloqueFisico* fisico = list_get(bloques_fisicos_gb, id_bloque_fisico);
                BloqueLogico* logico = crearBloqueLogico(i, fisico, tag->directorio);
                list_add(tag->bloques_logicos, logico);
            }
            log_debug(logger_storage,"(RECONSTRUCCION) - Bloques a reconstruir Tag %s: %d", tag->nombre_tag, string_array_size(bloques_logs_a_reconstruir));
            string_array_destroy(bloques_logs_a_reconstruir);
            list_add(file->tags,tag);
            log_debug(logger_storage,"(RECONSTRUCCION) - Tag Reconstruido: %s", tag->nombre_tag);
        }
        log_debug(logger_storage,"(RECONSTRUCCION) - Tags reconstruidos File %s: %d", file->nombre_file, list_size(file->tags));
        list_add(lista_files_gb, file);
        closedir(d_tags);
    }
    closedir(d_files);
    
}
#include "errores.h"


bool filePreexistente(char* nombre_file){
    File* file = buscarFilePorNombre(nombre_file); //para mi lo hizo Fer
    
    if (file != NULL){
        log_debug(logger_storage, "(filePreexistente) - FILE %s PREEXISTENTE",nombre_file);
        return true;
    }

    log_debug(logger_storage, "(filePreexistente) - Efectivamente el archivo %s no existia",nombre_file);

    return false;
}

bool tagPreexistente(t_list* tags,char* nombre_tag,char* nombre_file){
    Tag* tag = buscarTagPorNombre(tags,nombre_tag);

    if(tag != NULL){
        log_debug(logger_storage, "(tagPreexistente) - TAG %s EN FILE %s PREEXISTENTE",nombre_tag,nombre_file);
        return true;
    }

    return false;
}

bool fileInexistente(char* nombre_file){
    bool existe = filePreexistente(nombre_file);
    if (!existe) 
        log_debug(logger_storage, "(fileInexistente) - FILE %s INEXISTENTE",nombre_file);
    return !existe;
}

bool tagInexistente(t_list* tags,char* nombre_tag,char* nombre_file){
    bool existe = tagPreexistente(tags, nombre_tag, nombre_file);
    if (!existe) 
        log_debug(logger_storage, "(tagInexistente) - TAG %s EN FILE %s INEXISTENTE",nombre_tag,nombre_file);
    
    return !existe;
    
}

/*bool espacioInsuficiente(){

}*/

bool tieneEstadoCOMMITED(Tag* tag_a_consultar){
    char* estado_commit = config_get_string_value(tag_a_consultar->metadata_config_tag,"ESTADO"); 
    
    if (string_contains(estado_commit,"COMMITED"))
        return true;

    return false;
}

/*bool lecturaOEscrituraFueraDeLimite(){

}*/
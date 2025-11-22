#include "errores.h"


bool filePreexistente(char* nombre_file){
    File* file = buscarFilePorNombre(nombre_file) == NULL;
    
    if (file != NULL){
        log_debug(logger_storage, "(filePreexistente) - FILE %s PREEXISTENTE",nombre_file);
        return true;
    }

    return false;
}

bool tagPreexistente(t_list* tags,char* nombre_tag,char* nombre_file){

    if(buscarTagPorNombre(tags,nombre_tag) != NULL){
        log_debug(logger_storage, "(fileTagPreexistente) - TAG %s EN FILE %s PREEXISTENTE",nombre_tag,nombre_file);
        return true;
    }

    return false;
}

bool fileInexistente(char* nombre_file){
    bool existe = filePreexistente(nombre_file);
    if (!existe) 
        log_debug(logger_storage, "(fileInexistente) - FILE %s INEXISTENTE",nombre_file);
    return existe;
}

bool tagInexistente(t_list* tags,char* nombre_tag,char* nombre_file){
    bool existe = tagPreexistente(tags, nombre_tag, nombre_file);
    if (!existe) 
        log_debug(logger_storage, "(tagInexistente) - TAG %s EN FILE %s INEXISTENTE",nombre_tag,nombre_file);
    
    return existe;
    
}

bool espacioInsuficiente(){

}

bool estructuraNoPermitida(){
    
}

bool lecturaOEscrituraFueraDeLimite(){

}
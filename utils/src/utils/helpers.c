#include "helpers.h"
#include <stdlib.h>

t_log* IniciarLogger(char* nombre_modulo,int nivel_log)
{
    t_log* nuevo_logger;
    
    char* nombre_log_final = string_new();
    string_append(&nombre_log_final, nombre_modulo);
    string_append(&nombre_log_final, ".log");

    nuevo_logger = log_create(nombre_log_final,nombre_modulo,true,nivel_log);
    if(nuevo_logger==NULL){
        abort();
    }
    return nuevo_logger;
}

t_config* IniciarConfig(char* nombre_config)
{
    t_config* nuevo_config;

    nuevo_config = config_create(nombre_config);
    if (nuevo_config == NULL) {
    
    abort();
    }
    return nuevo_config;
}

void ChequearArgs(int cant_args_ingresados,int limite_cant_args){
    if (cant_args_ingresados > limite_cant_args) {
        printf("Error - (main) - Cantidad de argumentos invalida");
        abort();
    } 
}
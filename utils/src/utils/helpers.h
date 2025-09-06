#ifndef HELPERS_H_
#define HELPERS_H_

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>

t_log* IniciarLogger(char* nombre_modulo,int nivel_log);
t_config* IniciarConfig(char* nombre_config); 
void ChequearArgs(int cant_args_ingresados,int limite_cant_args);

#endif /* HELPERS_H_ */
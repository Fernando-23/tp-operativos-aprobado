#ifndef ERRORES_H_
#define ERRORES_H_
#include "estructuras.h"
#include "operaciones.h"

bool filePreexistente(char* nombre_file);
bool tagPreexistente(t_list* tags,char* nombre_tag,char* nombre_file);
bool tagInexistente(t_list* tags,char* nombre_tag,char* nombre_file);

#endif /*ERRORES_H_*/
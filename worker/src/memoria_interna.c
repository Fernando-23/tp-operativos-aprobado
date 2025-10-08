#include "memoria_interna.h"
#include "helpers-worker.h"

void* memoria;
int* bitMap;
int cant_frames;

frame_t* lista_frames;

t_list* tabla_general;

void IniciarMemoria(tam_pag)
{
    memoria = malloc((size_t) config_worker->tam_memoria);
    int cant_frames = config_worker->tam_memoria / tam_pag;

    //inicializo bitmap
    bitMap = (int*) malloc(cant_frames * sizeof(int));
    for (int i = 0; i < cant_frames; i++) {
        bitMap[i] = 0; //esta libre
    }

    //inicializo lista de frames
    lista_frames = (frame_t*) malloc(cant_frames * sizeof(frame_t));
    for (int i = 0; i < cant_frames; i++) {
        lista_frames[i].nro_frame = i;
        lista_frames[i].entrada = NULL; 
    }
    puntero=0;
}

tabla_paginas_t* buscar_o_crear_tabla(char* file, char* tag) {
    for (int i = 0; i < list_size(tabla_general); i++) {
        tabla_paginas_t* t = list_get(tabla_general, i);
        if (string_equals_ignore_case(t->file, file) &&
            string_equals_ignore_case(t->tag, tag)) {
            return t; // devuelvo la tabla existente
        }
    }
    // si no existe creo la tabla
    tabla_paginas_t* nueva = malloc(sizeof(tabla_paginas_t));
    nueva->file = strdup(file);
    nueva->tag = strdup(tag);
    nueva->entradas = NULL;
    nueva->cant_paginas = 0;

    list_add(tabla_general, nueva); //agrega la tabla a la tabla_general
    return nueva;
}







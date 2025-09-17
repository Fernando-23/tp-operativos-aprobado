#ifndef MEMORIA_INTERNA_H_
#define MEMORIA__INTERNA_H_

typedef struct {
    char* file;
    char* tag;
    entrada_pagina_t* entradas;   // array dinámico de páginas
    int cant_paginas;
} tabla_paginas_t;

typedef struct {
    int nro_frame;       
    int bitPresencia;    // 1 si está en memoria, 0 si no
    int bitUso;           
    int bitModificado;    
} entrada_pagina_t;


typedef struct{
    renglon_memoria* memoria;
}memoria_interna;

typedef struct{
    int nro_pag;
    char* contenido;
}renglon_memoria;

extern void *memoria;

void IniciarMemoria(tam_pag);


#endif /* MEMORIA_INTERNA_H_ */

/* memoria.h */


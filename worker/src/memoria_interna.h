#ifndef MEMORIA_INTERNA_H_
#define MEMORIA__INTERNA_H_

typedef struct {
    char* file;
    char* tag;
    entrada_pagina_t* entradas;   // array dinámico de páginas
    int cant_paginas;
} tabla_paginas_t;

typedef struct {
    int nro_pag;
    int nro_frame;       
    int bitPresencia;    // 1 si está en memoria, 0 si no
    int bitUso;           
    int bitModificado;
    
} entrada_pagina_t;

typedef struct {
    int nro_frame;       // Número de frame en memoria
    int nro_pag;         // Número de página que contiene
    int bitUso;          // Bit de uso para la política de reemplazo
    int bitModificado;
   // Bit de modificado para la política de reemplazo
} frame_t;

extern frame_t* lista_frames; // Array de frames

extern int* bitMap;
extern int cant_frames;
extern t_list* tabla_general;

extern void *memoria;

void IniciarMemoria(tam_pag);
tabla_paginas_t* buscar_o_crear_tabla(char* file, char* tag);




#endif /* MEMORIA_INTERNA_H_ */

/* memoria.h */


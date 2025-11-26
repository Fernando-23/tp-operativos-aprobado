#ifndef MEMORIA_INTERNA_H_
#define MEMORIA__INTERNA_H_

typedef struct {
    char* file;
    char* tag;
    t_list* entradas;   
    int cant_paginas;
} tabla_paginas_t;

typedef struct frame {
    int nro_frame;
    entrada_pagina_t* entrada; // <- puntero a la única entrada real
} frame_t;

typedef struct entrada_pagina {
    int nro_pag;                    // id bloque logico
    int nro_frame;                  // -1 si no residente
    uint8_t bitPresencia;           // P
    uint8_t bitUso;                 // U
    uint8_t bitModificado;          // M
    tabla_paginas_t* tabla;         // a la tabla original
    uint64_t last_used_ms;          // última vez que se usó (READ/WRITE o page-in)
} entrada_pagina_t;


extern frame_t* lista_frames; // Array de frames

extern int* bitMap;
extern int cant_frames;
extern t_list* tabla_general;
extern int puntero;
extern void *memoria;
extern pthread_mutex_t frame_modificado;

void iniciarMemoria();
tabla_paginas_t* buscarOCrearTabla(char* file, char* tag);
tabla_paginas_t* buscarTablaPags(char* file, char* tag);
entrada_pagina_t* buscarEntradaPorNroPag(t_list* entradas,int nro_pag);


#endif /* MEMORIA_INTERNA_H_ */

/* memoria.h */

/*

file:tag 

crear nueva tabla de paginas



*/
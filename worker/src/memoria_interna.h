#ifndef MEMORIA_INTERNA_H_
#define MEMORIA__INTERNA_H_

typedef struct {
    char* file;
    char* tag;
    t_list* entradas;   
    int cant_paginas;
} TablaPaginas; //tabla_paginas_t

typedef struct frame {
    int nro_frame;
    EntradaDeTabla* entrada; // <- puntero a la única entrada real
} Frame;

typedef struct entrada_pagina {
    int nro_pag;                    // id bloque logico
    int nro_frame; // nro Frame                  // -1 si no residente
    uint8_t bit_presencia;           // P
    uint8_t bit_uso;                 // U
    uint8_t bit_modificado;          // M
    TablaPaginas* tabla;         // a la tabla original
    uint64_t last_used_ms;          // última vez que se usó (READ/WRITE o page-in)
} EntradaDeTabla;


extern Frame* lista_frames; // Array de frames

extern int* bitMap;
extern int cant_frames;
extern t_list* tabla_general;
extern int puntero;
extern void *memoria;
extern pthread_mutex_t frame_modificado;

void iniciarMemoria();
TablaPaginas* buscarOCrearTabla(char* file, char* tag);
TablaPaginas* buscarTablaPags(char* file, char* tag);
EntradaDeTabla* buscarEntradaPorNroPag(t_list* entradas,int nro_pag); //busca por list_find
EntradaDeTabla *buscarEntradaPagina(TablaPaginas *tabla, int pag_actual); // busca por list_size
Frame* buscarFrameLibre();
int buscarFrameLibreEnBitmap();
void vaciarFrame(Frame* f);
Frame* elegirVictimaLRU();

#endif /* MEMORIA_INTERNA_H_ */

/* memoria.h */

/*

file:tag 

crear nueva tabla de paginas



*/
#ifndef MEMORIA_INTERNA_H_
#define MEMORIA_INTERNA_H_
#define _POSIX_C_SOURCE 200809L

#include "helpers-worker.h"
#include <time.h>

typedef struct TablaPaginas{
    char* file;
    char* tag;
    t_list* entradas;   
    int cant_paginas;
} TablaPaginas; //tabla_paginas_t

typedef struct EntradaDeTabla {
    int nro_pag;                    // id bloque logico
    int nro_frame; // nro Frame                  // -1 si no residente
    uint8_t bit_presencia;           // P
    uint8_t bit_uso;                 // U
    uint8_t bit_modificado;          // M
    TablaPaginas* tabla;         // a la tabla original
    uint64_t last_used_ms;          // última vez que se usó (READ/WRITE o page-in)
}EntradaDeTabla;


typedef struct{
    int tabla_index;
    int entrada_index;
    EntradaDeTabla* entrada;
}RespuestaAlgoritmoReemplazo;



extern char* error_en_operacion;
extern int* bitMap;
extern int cant_frames;
// extern t_list* lista_frames; // Array de frames
extern t_list* tabla_general;
extern int puntero;
extern void *memoria;
extern pthread_mutex_t frame_modificado;



void iniciarMemoria();

TablaPaginas* buscarTablaPags(char* file, char* tag);
TablaPaginas* buscarOCrearTabla(char* file, char* tag);
EntradaDeTabla *buscarOCrearEntradaPag(TablaPaginas *tabla_a_consultar, int pag_actual, char *file, char *tag);
EntradaDeTabla* buscarEntradaPagina(TablaPaginas* tabla, int pag_actual);
int buscarFrameLibreEnBitmap();
EntradaDeTabla* buscarEntradaPorNroPag(t_list* entradas,int nro_pag);


bool estaPagEnMemoria(char* file, char* tag, int nro_pag);
bool escribirEnMemoria(char* file, char* tag, int pagina, int desplazamiento,char* contenido);

char* leerEnMemoria(char* file, char* tag, int pagina, int desplazamiento, int tamanio);

int aplicarPoliticaReemplazo(char* file, char* tag, int nro_pag);

RespuestaAlgoritmoReemplazo* cargarRespuestaAlgoritmoRemplazo(int id_tabla, int id_entrada, EntradaDeTabla* entrada);
RespuestaAlgoritmoReemplazo* cicloClockM(int resetear_bit_uso, int bit_uso, int bit_modificado);
RespuestaAlgoritmoReemplazo* elegirVictimaLRU();

uint64_t now_ms(void);

int gestionarPAGE_FAULT(char* file, char* tag, int nro_pagina);

EntradaDeTabla* crearEntradaPagina(int pag_a_asignar, TablaPaginas* tabla);

int devolverFrameLibre(char* file, char* tag, int nro_pagina);

int obtenerCantEntradasDeTabla(TablaPaginas* tabla_a_consultar);

void liberarTablaPaginas(TablaPaginas* tabla_a_liberar);

int gestionarBitModificado(RespuestaAlgoritmoReemplazo* resp,char* file, char* tag, int nro_pagina);

bool escribirEnStorage(EntradaDeTabla* entrada_a_persistir);

void escribirPagina(char* file, char* tag, int nro_pagina,int nro_frame, char* contenido);

char* leerBloque(EntradaDeTabla* entrada_pagina);

void hacerRetardo();

bool estaPagEnMemoria(char* file, char* tag, int nro_pag);

t_list* obtenerEntradasAFlushear(TablaPaginas* tabla_a_flush);

uint64_t dirFisica(int nro_frame, int desp_actual);

void normalizar_puntero_clockm();

#endif /* MEMORIA_INTERNA_H_ */

/* memoria.h */

/*

file:tag 

crear nueva tabla de paginas



*/
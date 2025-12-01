#ifndef BITMAP_H_
#define BITMAP_H_
#include <sys/mman.h>
#include "estructuras.h"
#include "operaciones.h"

void inicializarBitmapYMapeo();
RespuestaConsultaBitmap* consultarBitmapPorBloquesLibres(int cant_bloques_que_quiero);
void liberarBitmapYMapeo();
void limpiarBitmap();


#endif /*BITMAP_H_*/
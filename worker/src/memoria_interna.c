#define _POSIX_C_SOURCE 200809L
#include "memoria_interna.h"
#include "helpers-worker.h"
#include <time.h>

void* memoria;
int* bitMap;
int cant_frames;

frame_t* lista_frames;

t_list* tabla_general;

int socket_master;
int socket_storage;

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
            return t; 
        }
    }
    
    tabla_paginas_t* nueva = malloc(sizeof(tabla_paginas_t));
    nueva->file = strdup(file);
    nueva->tag = strdup(tag);
    nueva->entradas = NULL;
    nueva->cant_paginas = 0;

    list_add(tabla_general, nueva); //agrega la tabla a la tabla_general
    return nueva;
}

int escribir_en_memoria_paginada(char* file, char* tag, int pagina, int desplazamiento, char* contenido)
{
    if (!contenido) return -1;
    if (pagina < 0 || desplazamiento < 0 || desplazamiento >= tam_pag) {
        log_error(logger_worker, "Parametros invalidos (pagina=%d, desp=%d)", pagina, desplazamiento);
        return -1;
    }

    size_t bytesContenido  = strlen(contenido);
    size_t totalAEscribir  = bytesContenido + 1; 
    size_t escritos        = 0;
    int    pag_actual      = pagina;
    int    desp_actual     = desplazamiento;

    tabla_paginas_t* tabla = buscar_o_crear_tabla(file, tag);
    if (!tabla) {
        log_error(logger_worker, "No se pudo obtener/crear tabla de paginas");
        return -1;
    }

    while (escritos < totalAEscribir) {
        entrada_pagina_t* ent = buscar_o_crear_entrada_pagina(tabla, pag_actual, file, tag);
        if (!ent) {
            log_error(logger_worker, "Fallo al obtener/crear entrada de pagina %d", pag_actual);
            return -1;
        }

        char* base_frame       = (char*)memoria + ((size_t)ent->nro_frame * (size_t)tam_pag);
        size_t espacioLibre    = (size_t)tam_pag - (size_t)desp_actual;
        size_t por_copiar      = totalAEscribir - escritos;
        if (por_copiar > espacioLibre) por_copiar = espacioLibre;

        // Copia del tramo que cae en ESTA página
        memcpy(base_frame + desp_actual, contenido + escritos, por_copiar);

       
        ent->bitUso        = 1;
        ent->bitModificado = 1;
        ent->last_used_ms  = now_ms();

       
        uint64_t dfis = dir_fisica(ent->nro_frame, desp_actual);
        
        log_info(logger_worker,
                 "Query %d: Acción: ESCRIBIR - Dirección Física: %llu - Valor: %.*s",
                 query->id_query,
                 (unsigned long long)dfis,
                 (const char*)(contenido + escritos));

        escritos    += por_copiar;

        // ¿Siguiente página?
        if (por_copiar == espacioLibre && escritos < totalAEscribir) {
            pag_actual++;
            desp_actual = 0;
        } else {
            desp_actual += (int)por_copiar;
        }
    }

    log_info(logger_worker,
             "Escritura completada: inicio pag=%d desp=%d, fin pag=%d, bytes=%zu",
             pagina, desplazamiento, pag_actual, totalAEscribir);

    return 0;
}

char* leer_en_memoria_paginada(char* file, char* tag, int pagina, int desplazamiento, int tamanio)
{
    if (tamanio <= 0 || pagina < 0 || desplazamiento < 0 || desplazamiento >= tam_pag) {
        log_error(logger_worker, "Parametros invalidos (pagina=%d, desp=%d, tam=%d)", pagina, desplazamiento, tamanio);
        return NULL;
    }

    char* mensaje = (char*)malloc((size_t)tamanio + 1u);
    if (!mensaje) {
        log_error(logger_worker, "Fallo al reservar memoria para lectura (%d bytes)", tamanio);
        return NULL;
    }

    size_t bytesObjetivo = (size_t)tamanio;
    size_t bytesLeidos   = 0;
    int    pag_actual    = pagina;
    int    desp_actual   = desplazamiento;

    tabla_paginas_t* tabla = buscar_o_crear_tabla(file, tag);
    if (!tabla) {
        log_error(logger_worker, "No se pudo obtener/crear tabla de paginas");
        free(mensaje);
        return NULL;
    }

    while (bytesLeidos < bytesObjetivo) {
        entrada_pagina_t* ent = buscar_o_crear_entrada_pagina(tabla, pag_actual, file, tag);
        if (!ent) {
            log_error(logger_worker, "No se encontro/creo la entrada de pagina %d", pag_actual);
            free(mensaje);
            return NULL;
        }
        if (ent->nro_frame == -1) {
            log_error(logger_worker, "Pagina %d no residente (nro_frame == -1). Falta traer de storage.", pag_actual);
            free(mensaje);
            return NULL;
        }

        char* base_frame        = (char*)memoria + ((size_t)ent->nro_frame * (size_t)tam_pag);
        size_t espacioDisponible= (size_t)tam_pag - (size_t)desp_actual;
        size_t por_copiar       = bytesObjetivo - bytesLeidos;
        if (por_copiar > espacioDisponible) por_copiar = espacioDisponible;

        // Copiamos este tramo desde la página actual
        memcpy(mensaje + bytesLeidos, base_frame + desp_actual, por_copiar);

      
        ent->bitUso       = 1;
        ent->last_used_ms = now_ms();

        
        uint64_t dfis = dir_fisica(ent->nro_frame, desp_actual);
        log_info(logger_worker,
                 "Query %d: Acción: LEER - Dirección Física: %llu - Valor: %.*s",
                 query->id_query,
                 (unsigned long long)dfis,
                 (int)por_copiar,
                 (const char*)(base_frame + desp_actual));

        bytesLeidos += por_copiar;

        // ¿Siguiente página?
        if (bytesLeidos < bytesObjetivo) {
            pag_actual++;
            desp_actual = 0;
        }
    }

    mensaje[bytesLeidos] = '\0';

    log_info(logger_worker,
             "Lectura completada: inicio pag=%d, fin pag=%d, bytes=%zu",
             pagina, pag_actual, bytesLeidos);

    return mensaje;
}

entrada_pagina_t *buscar_o_crear_entrada_pagina(tabla_paginas_t *tabla, int pag_actual, char *file, char *tag)
{
    entrada_pagina_t *entrada = buscar_entrada_pagina(tabla, pag_actual);
    // no existe la entrada, creo un ENTRADA
    if (entrada == NULL)
    {
        entrada = malloc(sizeof(entrada_pagina_t));
        if (entrada == NULL)
        {
            log_error(logger_worker, "No se pudo asignar memoria para nueva entrada de pagina");
            return NULL;
        }
        // busco frame libre ya que estoy creando una nueva entrada
        frame_t *frameLibre = buscar_frame_libre(file, tag);
        if (frameLibre == NULL)
        {
            log_error(logger_worker, "No se pudo obtener un frame libre para la nueva entrada de pagina");
            free(entrada);
            return NULL;
        }

        uint64_t t = now_ms();

        entrada->nro_pag = pag_actual;
        entrada->nro_frame = frameLibre->nro_frame;
        entrada->bitPresencia = 1;
        entrada->bitModificado = 0;
        entrada->bitUso = 1;
        entrada->tabla = tabla;

        entrada->last_used_ms = t;

        frameLibre->entrada = entrada;
        bitMap[frameLibre->nro_frame] = 1;

        list_add(tabla->entradas, entrada);

        frameLibre->entrada = entrada; 
    }
    return entrada;
}

entrada_pagina_t *buscar_entrada_pagina(tabla_paginas_t *tabla, int pag_actual)
{
    for (int i = 0; i < list_size(tabla->entradas); i++)
    {
        entrada_pagina_t *entrada = list_get(tabla->entradas, i);
        if (entrada->nro_pag == pag_actual)
        {
            return entrada;
        }
    }
    return NULL; // No encontrada
}
frame_t* buscar_frame_libre()
{
    for (int i = 0; i < cant_frames; i++) {
        if (bitMap[i] == 0) {
            bitMap[i] = 1;
            return &lista_frames[i];
        }
    }

    // Abi borre el comentario perdonameeeeeeee    
    frame_t *victima = aplicar_politica_reemplazo();
    if (!victima) return NULL;

    vaciar_frame(victima);     
    bitMap[victima->nro_frame] = 1;
    return victima;
}


frame_t *aplicar_politica_reemplazo(void)
{
    const char *algoritmo = config_worker->algoritmo_reemplazo;

    if (strcmp(algoritmo, "CLOCK-M") == 0) {
        frame_t *v = NULL;
        v = CicloCLockM(0, 0, 0); if (v) return v;
        v = CicloCLockM(1, 0, 1); if (v) return v;
        v = CicloCLockM(0, 0, 0); if (v) return v;  
        v = CicloCLockM(0, 0, 1); if (v) return v;

        log_error(logger_worker, "CLOCK-M no encontró víctima");
        return NULL;
    }
    else if (strcmp(algoritmo, "LRU") == 0) {
        frame_t *v = elegir_victima_lru();
        return v; 
    }
    else {
        log_error(logger_worker, "No se ingreso un algoritmo valido");
        return NULL;
    }
}


frame_t *CicloCLockM(int reset_u, int valor_uso, int valor_modificado)
{
    if (cant_frames <= 0 || !lista_frames) return NULL;

    // 1) de puntero a fin
    for (int i = puntero; i < cant_frames; i++) {
        frame_t *f = &lista_frames[i];
        entrada_pagina_t *e = f->entrada;
        if (!e) continue;

        if (e->bitUso == valor_uso && e->bitModificado == valor_modificado) {
            puntero = i + 1 ;
            if(puntero == cant_frames) {
                puntero=0;
            }
            return f;                          
        }
        if (reset_u) e->bitUso = 0;            
    }

    // 2) de 0 a puntero
    for (int i = 0; i < puntero; i++) {
        frame_t *f = &lista_frames[i];
        entrada_pagina_t *e = f->entrada;
        if (!e) continue;

        if (e->bitUso == valor_uso && e->bitModificado == valor_modificado) {
            puntero = i + 1 ;
            if(puntero == cant_frames) {
                puntero=0;
            }
            return f;
        }
        if (reset_u) e->bitUso = 0;
    }

    return NULL;
}

 frame_t* elegir_victima_lru(void)
{
    frame_t* victima = NULL;
    uint64_t oldest  = UINT64_MAX;

    for (int i = 0; i < cant_frames; i++) {
        frame_t* f = &lista_frames[i];
        entrada_pagina_t* e = f->entrada;
        if (!e || e->bitPresencia == 0) continue;

        if (e->last_used_ms < oldest) {
            oldest  = e->last_used_ms;
            victima = f;
        }
    }
    return victima;
}

static void vaciar_frame(frame_t *f)
{
   
    entrada_pagina_t *e = f->entrada;
    if (e) {
       
        if (e->bitModificado) {
            // Tomamos file/tag desde la propia tabla
            enviarFrameModificadoStorage(f, e->tabla->file, e->tabla->tag);
            e->bitModificado = 0;
        }

        
        e->bitPresencia = 0;
        e->bitUso       = 0;
        e->nro_frame    = -1;
        e->last_used_ms = 0;
    }

    // liberar frame en el bitmap y desvincular
    bitMap[f->nro_frame] = 0;
    f->entrada = NULL;
    
}


void enviarFrameModificadoStorage(frame_t* frame, char* file, char* tag){
    char* base_frame = (char*)memoria + ((size_t)frame->nro_frame * (size_t)tam_pag);

    char* contenidoPagina = malloc((size_t)tam_pag + 1);
    memcpy(contenidoPagina, base_frame, (size_t)tam_pag);
    contenidoPagina[tam_pag] = '\0';

    pthread_mutex_lock(&frame_modificado);
    Mensaje* mensajito = malloc(sizeof(Mensaje));
    mensajito->mensaje = contenidoPagina;
    mensajito->size = tam_pag + 1;
    EnviarString(mensajito,socket_storage,logger_worker);
    pthread_mutex_unlock(&frame_modificado);
    //Liberamos porque se envia a storage y me chupa un huevo lo que hace :)
    free(contenidoPagina);
}

uint64_t now_ms(void){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts); // evita cambios de hora del sistema
    return (uint64_t)ts.tv_sec * 1000ull + (uint64_t)ts.tv_nsec / 1000000ull;
}

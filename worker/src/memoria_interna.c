#include "memoria_interna.h"

void* memoria; // 
int* bitMap;
int cant_frames;

Frame* lista_frames;

t_list* tabla_general;

int socket_master;
int socket_storage;

void iniciarMemoria(){
    memoria = malloc((size_t) config_worker->tam_memoria);
    int cant_frames = config_worker->tam_memoria / tam_pag;

    //inicializo bitmap
    bitMap = malloc(cant_frames * sizeof(int));
    for (int i = 0; i < cant_frames; i++) {
        bitMap[i] = 0; //esta libre
    }

    //inicializo lista de frames
    lista_frames = (Frame*) malloc(cant_frames * sizeof(Frame));
    for (int i = 0; i < cant_frames; i++) {
        lista_frames[i].nro_frame = i;
        lista_frames[i].entrada = NULL; 
    }
    puntero = 0;
}


TablaPaginas* buscarTablaPags(char* file, char* tag){
    for (int i = 0; i < list_size(tabla_general); i++) {
        TablaPaginas* t = list_get(tabla_general, i);
        if (string_equals_ignore_case(t->file, file) &&
            string_equals_ignore_case(t->tag, tag)) {
            return t; 
        }
    }
    return NULL;
}

TablaPaginas* buscarOCrearTabla(char* file, char* tag) {

    TablaPaginas* tabla = buscarTablaPags( file, tag);

    if(tabla == NULL){
        tabla = malloc(sizeof(TablaPaginas));
        tabla->file = string_duplicate(file);
        tabla->tag = string_duplicate(tag);
        tabla->entradas = list_create();
        tabla->cant_paginas = 0;

    list_add(tabla_general, tabla);
    }
    return tabla;
}

/*  

    for()


*/
int escribir_en_memoria_paginada(char* file, char* tag, int pagina, int desplazamiento, char* contenido){
     
    if (!contenido) 
        return -1;
    if (pagina < 0 || desplazamiento < 0 || desplazamiento >= tam_pag) {
        log_error(logger_worker, "Parametros invalidos (pagina=%d, desp=%d)", pagina, desplazamiento);
        return -1;
    }

    size_t bytesContenido = strlen(contenido);
    size_t totalAEscribir = bytesContenido + 1; 
    size_t escritos = 0;
    int pag_actual = pagina;
    int desp_actual = desplazamiento;

    TablaPaginas* tabla = buscarOCrearTabla(file, tag);
    if (!tabla) {
        log_error(logger_worker, "No se pudo obtener/crear tabla de paginas");
        return -1;
    }

    while (escritos < totalAEscribir) {
        EntradaDeTabla* ent = buscar_o_crear_entrada_pagina(tabla, pag_actual, file, tag);
        if (!ent) {
            log_error(logger_worker, "Fallo al obtener/crear entrada de pagina %d", pag_actual);
            return -1;
        }

        char* base_frame       = (char*)memoria + ((size_t)ent->nro_frame * (size_t)tam_pag);
        size_t espacioLibre    = (size_t)tam_pag - (size_t)desp_actual;
        size_t por_copiar      = totalAEscribir - escritos; // Out en nueva func
        if (por_copiar > espacioLibre) por_copiar = espacioLibre; 

        // Copia del tramo que cae en ESTA página
        memcpy(base_frame + desp_actual, contenido + escritos, por_copiar);

       
        ent->bit_uso        = 1;
        ent->bit_modificado = 1;
        ent->last_used_ms  = now_ms();

       
        uint64_t dfis = dir_fisica(ent->nro_frame, desp_actual);
        
        log_info(logger_worker,
                 "Query %d: Acción: ESCRIBIR - Dirección Física: %llu - Valor: %.*s",
                 query->id_query,
                 (unsigned long long)dfis,
                 (const char*)(contenido + escritos));

        escritos += por_copiar;

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

    TablaPaginas* tabla = buscarOCrearTabla(file, tag);

    while (bytesLeidos < bytesObjetivo) {
        EntradaDeTabla* ent = buscar_o_crear_entrada_pagina(tabla, pag_actual, file, tag);
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

      
        ent->bit_uso       = 1;
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


EntradaDeTabla *buscar_o_crear_entrada_pagina(TablaPaginas *tabla, int pag_actual, char *file, char *tag)
{
    EntradaDeTabla *entrada = buscar_entrada_pagina(tabla, pag_actual);
    if (entrada == NULL){
        entrada = malloc(sizeof(EntradaDeTabla));
        if (entrada == NULL){
            log_error(logger_worker, "(buscar_o_crear_entrada_pagina) - Fallo malloc");
            return NULL;
        }
        // busco frame libre ya que estoy creando una nueva entrada
        Frame *frameLibre = buscar_frame_libre(file, tag);
        if (frameLibre == NULL){
            log_error(logger_worker, "No se pudo obtener un frame libre para la nueva entrada de pagina");
            free(entrada);
            return NULL;
        }

        uint64_t t = now_ms();

        entrada->nro_pag = pag_actual;
        entrada->nro_frame = frameLibre->nro_frame;
        entrada->bit_presencia = 1;
        entrada->bit_modificado = 0;
        entrada->bit_uso = 1;
        entrada->tabla = tabla;

        entrada->last_used_ms = t;

        frameLibre->entrada = entrada;  //chauuu
        bitMap[frameLibre->nro_frame] = 1;  //chau dos

        list_add(tabla->entradas, entrada);

        frameLibre->entrada = entrada; 
    }
    else{
        Frame *frameLibre = buscarFrameLibre(file, tag);
        entrada->bit_presencia = 1;
    }
    return entrada;
}

EntradaDeTabla* buscarEntradaPagina(TablaPaginas* tabla, int pag_actual){
    
    for (int i = 0; i < list_size(tabla->entradas); i++)
    {
        EntradaDeTabla *entrada = list_get(tabla->entradas, i);
        if (entrada->nro_pag == pag_actual)
        {
            return entrada;
        }
    }
    return NULL; // No encontrada
}


int buscarFrameLibreEnBitmap(){
    // pthread_mutex_lock(mx_bitmap);
    for (int i = 0; i < cant_frames; i++) {
        if (bitMap[i] == 0) {
            bitMap[i] = 1;
            //mutex bitmap unlock
            return i;
        }
    }
    // pthread_mutex_unlock(mx_bitmap);
    return -1; // no encontro bitmap libre
}



Frame* buscarFrameLibre(){
    for (int i = 0; i < cant_frames; i++) {
        if (bitMap[i] == 0) {
            bitMap[i] = 1;
            return &lista_frames[i];
        }
    }

    // Abi borre el comentario perdonameeeeeeee    
    Frame *victima = aplicar_politica_reemplazo();
    if (!victima) return NULL;

    vaciarFrame(victima);     
    bitMap[victima->nro_frame] = 1;
    return victima;
}


int aplicarPoliticaReemplazo(){
    const char *algoritmo = config_worker->algoritmo_reemplazo;
    int frame_a_retornar = -1;
    RespuestaAlgoritmoReemplazo* resp = NULL;
    if (string_equals_ignore_case(algoritmo, "CLOCK-M")) {
        
        resp = cicloClockM(0, 0, 0); if (resp != NULL) return gestionarBitModificado(resp);
        resp = cicloClockM(1, 0, 1); if (resp != NULL) return gestionarBitModificado(resp);
        resp = cicloClockM(0, 0, 0); if (resp != NULL) return gestionarBitModificado(resp);
        resp = cicloClockM(0, 0, 1); if (resp != NULL) return gestionarBitModificado(resp);

        log_error(logger_worker, "(aplicarPoliticaReemplazo) - CLOCK-M no encontró víctima");
        return NULL;
    }
    else if (string_equals_ignore_case(algoritmo, "LRU") == 0) {
        resp = elegirVictimaLRU();return gestionarBitModificado(resp); 
    }
    else {
        log_error(logger_worker, "No se ingreso un algoritmo valido");
        return NULL;
    }
}

RespuestaAlgoritmoReemplazo* cargarRespuestaAlgoritmoRemplazo(int id_tabla, int id_entrada, EntradaDeTabla* entrada){
    RespuestaAlgoritmoReemplazo* resp = malloc(sizeof(RespuestaAlgoritmoReemplazo));
    resp->entrada = entrada;
    resp->tabla_index = id_tabla;
    resp->entrada_index = id_entrada;
    return resp;
}


RespuestaAlgoritmoReemplazo* cicloClockM(int resetear_bit_uso, int bit_uso, int bit_modificado){
    
    int id_entrada;
    int id_tabla;
    
    // 1) de puntero a fin
    for(id_tabla = ptr_gb.nro_tabla; id_tabla < list_size(tabla_general);id_tabla++){ 
        TablaPaginas* tabla_selec = (TablaPaginas *)list_get(tabla_general,id_tabla);
        
        for(id_entrada = ptr_gb.nro_entrada; id_entrada < list_size(tabla_selec->entradas); id_entrada++){
            EntradaDeTabla* entrada = (EntradaDeTabla *)list_get(tabla_selec->entradas, id_entrada);

            if (entrada->bit_presencia == 1 && entrada->bit_uso == bit_uso && entrada->bit_modificado == bit_modificado) {
                id_entrada++;
                
                if(id_entrada == list_size(tabla_selec->entradas)) { 
                    id_entrada = 0;
                    id_tabla++;
                    //TRAIGAN TUCO QUE EL SPAGUETTI YA ESTA SERVIDO
                    if(id_tabla == list_size(tabla_general)) // volvi abi de Peru
                        id_tabla = 0;
                }

                ptr_gb.nro_entrada = id_entrada;
                ptr_gb.nro_tabla = id_tabla;
                
                return cargarRespuestaAlgoritmoRemplazo(id_tabla,id_entrada,entrada);                         
            }
            
            if (resetear_bit_uso) 
                entrada->bit_uso = 0;     
        }
        id_entrada = 0;
    }

    id_tabla = 0;
    
    for(id_tabla = 0; id_tabla <= ptr_gb.nro_tabla; id_tabla++){
        TablaPaginas* tabla_selec = (TablaPaginas *)list_get(tabla_general,id_tabla);

        for(id_entrada = 0; id_entrada < list_size(tabla_selec->entradas);id_entrada++){
            EntradaDeTabla* entrada = (EntradaDeTabla *)list_get(tabla_selec->entradas, id_entrada);

            if(id_tabla == ptr_gb.nro_tabla && id_entrada == ptr_gb.nro_entrada){
                return NULL;
            }
            
            if (entrada->bit_presencia == 1 && entrada->bit_uso == bit_uso && entrada->bit_modificado == bit_modificado) {
                id_entrada++;
                
                if(id_entrada == list_size(tabla_selec->entradas)) { 
                    id_entrada = 0;
                    id_tabla++;
                    //TRAIGAN TUCO QUE EL SPAGUETTI YA ESTA SERVIDO OTRA VEZ
                    if(id_tabla == list_size(tabla_general)) // volvi abi de Peru
                        id_tabla = 0;
                }

                ptr_gb.nro_entrada = id_entrada;
                ptr_gb.nro_tabla = id_tabla;
                return cargarRespuestaAlgoritmoRemplazo(id_tabla,id_entrada,entrada);                          
            }
            
            if (resetear_bit_uso) 
                entrada->bit_uso = 0;   
            
            
        }
        id_entrada = 0;
    }
}

RespuestaAlgoritmoReemplazo* elegirVictimaLRU(){
    uint64_t vistima_elegida = UINT64_MAX; //q se vacha
    int id_tabla;
    int id_entrada;
    EntradaDeTabla* entrada_vistima;

    for(int i = 0; i < list_size(tabla_general);i++){ 
        TablaPaginas* tabla_selec = (TablaPaginas *)list_get(tabla_general,i);
        
        for(int j = 0; j < list_size(tabla_selec->entradas); j++){
            EntradaDeTabla* entrada = (EntradaDeTabla *)list_get(tabla_selec->entradas, j);
            if (entrada->bit_presencia == 1 && entrada->last_used_ms < vistima_elegida) {
                log_debug(logger_worker,"AGUS TE QUEREMOS MUCHO, FORRO - Victima elegida %d",j);
                vistima_elegida = entrada->last_used_ms;
                entrada_vistima = entrada;
                id_tabla = i;
                id_entrada = j;                          
            }
        }
    }
    return cargarRespuestaAlgoritmoRemplazo(id_tabla,id_entrada,entrada_vistima);
}

static void vaciarFrame(Frame* f){
   
    EntradaDeTabla *e = f->entrada;
    if (e) {
       
        if (e->bitModificado) {
            enviarFrameModificadoStorage(f, e->tabla->file, e->tabla->tag);
            e->bitModificado = 0;
        }

        e->nro_pag = -1;
        e->bitPresencia = 0;
        e->bitUso       = 0;
        e->nro_frame    = -1;
        e->last_used_ms = 0;
    }
    bitMap[f->nro_frame] = 0;
    f->entrada = NULL;
    
}

uint64_t now_ms(void){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts); // evita cambios de hora del sistema
    return (uint64_t)ts.tv_sec * 1000ull + (uint64_t)ts.tv_nsec / 1000000ull;
}

EntradaDeTabla* buscarEntradaPorNroPag(t_list* entradas,int nro_pag){

    bool tieneMismoNroPag(void *ptr){
        EntradaDeTabla* entrada = (EntradaDeTabla *)ptr;
        return (entrada->nro_pag == nro_pag);
    }
    return list_find(entradas, tieneMismoNroPag);
}

int gestionarPAGE_FAULT(char* file, char* tag, int nro_pagina){
    char* mensaje_formateado = string_from_format("LEER %s:%s %d" ,file, tag, nro_pagina); // storage lee toda la pagina y decime que tiene
    
    Mensaje* mensajito = crearMensajito(mensaje_formateado);
    enviarMensajito(mensajito, socket_storage, logger_worker);
    free(mensaje_formateado);
         
    Mensaje* mensaje_recibido = recibirMensajito(socket_storage, logger_worker); // ENUMSERRORSTORAGE CONTENIDO 
    char** datos_recibidos = string_split(mensaje_recibido->mensaje," "); //LIBERAR
    ErrorStorageEnum cod_op_storage_recv = atoi(datos_recibidos[0]);
    
    switch (cod_op_storage_recv){
        case OK:
            char* contenido_pagina = datos_recibidos[1];
            return devolverFrameLibre();

        case ESCRITURA_FUERA_DE_LIMITE:
        case ESCRITURA_NO_PERMITIDA:
        {
            log_debug(logger_worker,"(ejecutarWrite) - Error manejable recibido: %s", NOMBRE_ERRORES[cod_op_storage_recv]);
            break;
        }
            
        default: log_error(logger_worker, "Storage mando cualquier cosa, Cosa que mando: %s", cod_op_storage_recv);
        }
 }


EntradaDeTabla* crearEntradaPagina(int pag_a_asignar, TablaPaginas* tabla){ // tercer parametro cuestionable
    EntradaDeTabla* entrada_tabla = malloc(sizeof(EntradaDeTabla));

    uint64_t timestamp = now_ms();
    entrada_tabla->nro_pag = pag_a_asignar;
    entrada_tabla->nro_frame = -1;
    entrada_tabla->bit_presencia = 0;
    entrada_tabla->bit_modificado = 0;
    entrada_tabla->bit_uso = 1;
    entrada_tabla->tabla = tabla;
    entrada_tabla->last_used_ms = timestamp;
    
    return entrada_tabla;
}

//HICISTE TODAS LAS CHANCHADAS
int devolverFrameLibre(){

    // entrada la creamos siempre
    // tenemos que asignarle un marco a la entrada creada para escribir el contenido
    // puede haber marco disponible o no (bitmap)
    // caso feliz: asignamos el frame y escribimos contenido en ese frame
    // caso llorona: elegimos una victima, y vemos si esta modificado
    // caso llorona feliz: no mandamos nada a storage y le asignamos el marco a nuestra entrada y escribimos
    // caso llorona llorona: mandamos write a storage de pag victima y dsp asignamos el marco a nuesta entrada y escribimos

    int frame = buscarFrameLibreEnBitmap();
    if (frame != -1){
        log_debug(logger_worker,"(gestionarAsignaccionFrame) - Frame libre en bitmap: %d, caso lindo",frame);
        return frame;
    }
    
    log_debug(logger_worker,"(gestionarAsignaccionFrame) - No hubo frame libre, aplicando LA MATANZA");
    frame = aplicarPoliticaReemplazo();
    
    log_debug(logger_worker,"(gestionarAsignaccionFrame) - Frame seleccionado %d",frame);
    return frame;   
}

int obtenerCantEntradasDeTabla(TablaPaginas* tabla_a_consultar){
    return list_size(tabla_a_consultar->entradas);
}

void liberarTablaPaginas(TablaPaginas* tabla_a_liberar){
    free(tabla_a_liberar->file);
    free(tabla_a_liberar->tag);
    free(tabla_a_liberar);
}

int gestionarBitModificado(RespuestaAlgoritmoReemplazo* resp){

    if(resp->entrada->bit_modificado == 1){
        escribirEnStorage(resp->entrada); //TODO 
    } 
        
    TablaPaginas* tabla_padre = resp->entrada->tabla;

    EntradaDeTabla* entrada = list_remove(tabla_padre->entradas,resp->entrada_index);
    log_debug(logger_worker,"(aplicarPoliticaReemplazo) - Entrada borrada");
    int frame_a_retornar = entrada->nro_frame;    
    free(entrada);

    if (list_is_empty(tabla_padre->entradas)){
        log_debug(logger_worker,"(aplicarPoliticaReemplazo) - Tabla padre sin mas entradas, se procede a volarla");
        TablaPaginas* tabla_a_borrar = list_remove(tabla_general,resp->tabla_index);
        liberarTablaPaginas(tabla_a_borrar);
    }

    free(resp);
    return frame_a_retornar;
}

void escribirEnStorage(EntradaDeTabla* entrada_a_persistir){
    TablaPaginas* tabla_padre = entrada_a_persistir->tabla;
    char* contenido_a_persistir = 
        string_from_format("ESCRIBIR_BLOQUE %s %s:%s %s",query->id_query,tabla_padre->file,tabla_padre->tag,leerBloque(entrada_a_persistir));
    Mensaje* mensajito_a_enviar = crearMensajito(contenido_a_persistir); // ESCRIBIR_BLOQUE query_id file:tag contenido
    free(contenido_a_persistir); 
    enviarMensajito(mensajito_a_enviar,socket_storage,logger_worker);
    Mensaje* respuesta_storage = recibirMensajito(socket_storage,logger_worker); // OK siempre... ponele
    if(respuesta_storage == NULL || respuesta_storage->mensaje != "OK"){
        liberarMensajito(respuesta_storage);
        log_error(logger_worker, "(escribirEnStorage) Conexion cerrada con storage o no pudo escribir");
        exit(EXIT_FAILURE);
    }
    
    liberarMensajito(respuesta_storage);
}

char* leerBloque(EntradaDeTabla* entrada_pagina){ // memoria me desplazo hasta la base del bloque
    TablaPaginas* tabla_padre = entrada_pagina->tabla;

    int desplazamiento_hasta_base = entrada_pagina->nro_frame * tam_pag;
    char* base = memoria + desplazamiento_hasta_base;
    char* leido;
    memcpy(leido,base,tam_pag);
    log_debug(logger_worker,"(leerBloque) - File:Tag %s:%s, Contenido leido:",tabla_padre->file,tabla_padre->tag);
    log_debug(logger_worker,leido);
    return leido;
}

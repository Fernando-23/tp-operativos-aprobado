#include "memoria_interna.h"

void* memoria = NULL; // 
int* bitMap = NULL;
int cant_frames = 0;

char* error_en_operacion = "OK";

t_list* tabla_general = NULL;


void iniciarMemoria(){
    log_debug(logger_worker, "Iniciando memoria interna paginada...");
    memoria = malloc(config_worker->tam_memoria);
    if (!memoria) {
        log_error(logger_worker, "(iniciarMemoria) - Fallo malloc de memoria principal");
        exit(EXIT_FAILURE);
    }

    cant_frames = config_worker->tam_memoria / tam_pag;

    //inicializo bitmap
    bitMap = calloc(cant_frames, sizeof(int)); // Inicializa en 0 (todos libres)

    tabla_general = list_create();
    ptr_gb.nro_entrada = 0;
    ptr_gb.nro_tabla = 0;

    log_debug(logger_worker, "Memoria inicializada, %d frames de %d bytes", 
             cant_frames, tam_pag);
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
    log_debug(logger_worker,"(buscarOCrearTabla) - Buscando tabla de paginas para File: %s - Tag: %s",file,tag);
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

EntradaDeTabla *buscarOCrearEntradaPag(TablaPaginas *tabla_a_consultar, int pag_actual, char *file, char *tag){
    EntradaDeTabla* entrada = buscarEntradaPorNroPag(tabla_a_consultar->entradas, pag_actual);
    
    if (entrada == NULL){
        entrada = crearEntradaPagina(pag_actual, tabla_a_consultar);        
    }
    
    return entrada;    
}

EntradaDeTabla* buscarEntradaPagina(TablaPaginas* tabla, int pag_actual){
    log_debug(logger_worker,"(buscarEntradaPagina) - Buscando entrada de pagina %d en tabla de paginas File: %s - Tag: %s",pag_actual,tabla->file,tabla->tag);  
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
    log_debug(logger_worker,"(buscarFrameLibreEnBitmap) - Buscando frame libre en bitmap");
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

EntradaDeTabla* buscarEntradaPorNroPag(t_list* entradas,int nro_pag){
    log_debug(logger_worker,"(buscarEntradaPorNroPag) - Buscando entrada de pagina %d",nro_pag);
    bool tieneMismoNroPag(void *ptr){
        EntradaDeTabla* entrada = (EntradaDeTabla *)ptr;
        return (entrada->nro_pag == nro_pag);
    }
    return list_find(entradas, tieneMismoNroPag);
}


bool escribirEnMemoria(char* file, char* tag, int pagina, int desplazamiento,char* contenido){
    log_debug(logger_worker,"(escribirEnMemoria) - Escribiendo en memoria - File: %s - Tag: %s - Pagina: %d - Desplazamiento: %d - Contenido: %s",file,tag,pagina,desplazamiento,contenido);    
    size_t bytes_contenido = strlen(contenido);
    size_t total_a_escribir = bytes_contenido + 1; 
    size_t escritos = 0;
    int pag_actual = pagina;
    int desp_actual = desplazamiento;
    int frame_seleccionado;
    TablaPaginas* tabla_paginas = buscarOCrearTabla(file, tag);

    while(escritos < total_a_escribir){ 
        
        EntradaDeTabla* entrada_pag = buscarEntradaPagina(tabla_paginas, pag_actual);
        if (entrada_pag == NULL || entrada_pag->bit_presencia == 0){
            if(entrada_pag == NULL){ 
                entrada_pag = crearEntradaPagina(pag_actual,tabla_paginas);
                list_add(tabla_paginas->entradas,entrada_pag);   
                log_debug(logger_worker,"(escribirEnMemoria) - Se crea la entrada de pagina %d en tabla de paginas",pag_actual);    
            }
            log_info(logger_worker,"Query %d: - Memoria Miss - File: %s - Tag: %s - Pagina: %d",query->id_query,file,tag,pag_actual);
            frame_seleccionado = gestionarPAGE_FAULT(file,tag,pagina);
            if(frame_seleccionado == -1) return false; 
            entrada_pag->nro_frame = frame_seleccionado;
        }
        void* base_frame = memoria + (entrada_pag->nro_frame * tam_pag);
        size_t espacioLibre = tam_pag - desp_actual;
        size_t por_copiar = total_a_escribir - escritos; 
        if (por_copiar > espacioLibre) por_copiar = espacioLibre; 

        // Copia del tramo que cae en ESTA página
        memcpy(base_frame + desp_actual, contenido + escritos, por_copiar);

        entrada_pag->bit_uso = 1;
        entrada_pag->bit_modificado = 1;
        entrada_pag->last_used_ms = now_ms();

       
        uint64_t dfis = dirFisica(entrada_pag->nro_frame, desp_actual);
        
        log_info(logger_worker,
                 "Query %d: Acción: ESCRIBIR - Dirección Física: %llu - Valor Escrito: %s",
                 query->id_query,
                 (unsigned long long)dfis,
                 contenido);

        escritos += por_copiar;

        // ¿Siguiente página?
        if (por_copiar == espacioLibre && escritos < total_a_escribir) {
            pag_actual++;
            desp_actual = 0;
        } else {
            desp_actual += (int)por_copiar;
        }
        
    }
    return true;


}

void escribirPagina(char*file, char* tag, int nro_pagina,int nro_frame, char* contenido){
    char* base_frame = memoria + (nro_frame*tam_pag);
    memcpy(base_frame, contenido, 1);
    log_info(logger_worker,"Query %d: - Memoria Add - File: %s - Tag: %s - Pagina: %d - Marco: %d",query->id_query,file,tag,nro_pagina,nro_frame);                                                          
}

uint64_t dirFisica(int nro_frame, int desp_actual){
    return (uint64_t)(nro_frame*tam_pag) + desp_actual;
}

char* leerEnMemoria(char* file, char* tag, int pagina, int desplazamiento, int tamanio)
{   
    log_debug(logger_worker,"(leerEnMemoria) - Leyendo en memoria - File: %s - Tag: %s - Pagina: %d - Desplazamiento: %d - Tamaño: %d",file,tag,pagina,desplazamiento,tamanio);
    if (tamanio <= 0 || pagina < 0 || desplazamiento < 0 || desplazamiento >= tam_pag) {
        log_error(logger_worker, "(leer_en_memoria_paginada) - Parametros invalidos (pagina=%d, desp=%d, tam=%d)", pagina, desplazamiento, tamanio);
        return NULL;
    }

    char* mensaje = malloc(tamanio + 1);
    if (!mensaje) {
        log_error(logger_worker, "(leer_en_memoria_paginada) - Fallo al reservar memoria para lectura (%d bytes)", tamanio);
        return NULL;
    }

    size_t bytesObjetivo = tamanio;
    size_t bytesLeidos   = 0;
    int    pag_actual    = pagina;
    int    desp_actual   = desplazamiento;

    TablaPaginas* tabla = buscarOCrearTabla(file, tag);

    while (bytesLeidos < bytesObjetivo) {
        log_debug(logger_worker,"(leerEnMemoria) - Leyendo pagina %d, desplazamiento %d",pag_actual,desp_actual);
        EntradaDeTabla* ent = buscarOCrearEntradaPag(tabla, pag_actual, file, tag);
        if (!ent) {
            log_error(logger_worker, "No se encontro/creo la entrada de pagina %d", pag_actual);
            free(mensaje);
            return NULL;
        }
        
        if (ent->bit_presencia == 0) {
            log_debug(logger_worker,"(leerEnMemoria) - Page Fault en pagina %d",pag_actual);
            int frame_seleccionado = gestionarPAGE_FAULT(file,tag,pagina);
            if(frame_seleccionado == -1) return NULL;
            ent->nro_frame = frame_seleccionado;   
        }

        char* base_frame        = (char*)memoria + ((size_t)ent->nro_frame * (size_t)tam_pag);
        size_t espacioDisponible= (size_t)tam_pag - (size_t)desp_actual;
        size_t por_copiar       = bytesObjetivo - bytesLeidos;
        if (por_copiar > espacioDisponible) por_copiar = espacioDisponible;
        // Copiamos este tramo desde la página actual
        memcpy(mensaje + bytesLeidos, base_frame + desp_actual, por_copiar);

        ent->bit_uso       = 1;
        ent->last_used_ms = now_ms();

        uint64_t dfis = dirFisica(ent->nro_frame, desp_actual);
        log_info(logger_worker,
                 "Query %d: Acción: LEER - Dirección Física: %llu - Valor Leido: %s",
                 query->id_query,
                 (unsigned long long) dfis,
                 (const char*)(base_frame + desp_actual));

        bytesLeidos += por_copiar;

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



int aplicarPoliticaReemplazo(char* file, char* tag, int nro_pag){
    log_debug(logger_worker,"(aplicarPoliticaReemplazo) - Aplicando politica de reemplazo %s",config_worker->algoritmo_reemplazo);  

    RespuestaAlgoritmoReemplazo* resp = NULL;
    if (string_equals_ignore_case(config_worker->algoritmo_reemplazo, "CLOCK-M")) {
        
        resp = cicloClockM(0, 0, 0); if (resp != NULL) return gestionarBitModificado(resp,file,tag,nro_pag);
        resp = cicloClockM(1, 0, 1); if (resp != NULL) return gestionarBitModificado(resp,file,tag,nro_pag);
        resp = cicloClockM(0, 0, 0); if (resp != NULL) return gestionarBitModificado(resp,file,tag,nro_pag);
        resp = cicloClockM(0, 0, 1); if (resp != NULL) return gestionarBitModificado(resp,file,tag,nro_pag);

        log_error(logger_worker, "(aplicarPoliticaReemplazo) - CLOCK-M no encontró víctima");
        return -1;
    }
    else if (string_equals_ignore_case(config_worker->algoritmo_reemplazo, "LRU") == 0) {
        log_debug(logger_worker,"(aplicarPoliticaReemplazo) - Aplicando LRU");
        resp = elegirVictimaLRU();return gestionarBitModificado(resp,file,tag,nro_pag); 
    }
    else {
        log_error(logger_worker, "No se ingreso un algoritmo valido");
        return -1 ;
    }
}

RespuestaAlgoritmoReemplazo* cargarRespuestaAlgoritmoRemplazo(int id_tabla, int id_entrada, EntradaDeTabla* entrada){
    log_debug(logger_worker,"(cargarRespuestaAlgoritmoRemplazo) - Cargando respuesta algoritmo de reemplazo - Tabla: %d - Entrada: %d",id_tabla,id_entrada);
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

        //hacerRetardo();
        hacerRetardo();
        
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

        hacerRetardo();

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
    return NULL;
}

RespuestaAlgoritmoReemplazo* elegirVictimaLRU(){
    uint64_t vistima_elegida = UINT64_MAX; //q se vacha
    int id_tabla;
    int id_entrada;
    EntradaDeTabla* entrada_vistima;

    for(int i = 0; i < list_size(tabla_general);i++){ 
        TablaPaginas* tabla_selec = (TablaPaginas *)list_get(tabla_general,i);

        hacerRetardo();
        
        for(int j = 0; j < list_size(tabla_selec->entradas); j++){
            EntradaDeTabla* entrada = (EntradaDeTabla *)list_get(tabla_selec->entradas, j);
            if (entrada->bit_presencia == 1 && entrada->last_used_ms < vistima_elegida) {
                log_debug(logger_worker,"AGUS TE QUEREMOS MUCHO, FORRO. GRACIAS MUCHACHOS! - Victima elegida %d",j);
                vistima_elegida = entrada->last_used_ms;
                entrada_vistima = entrada;
                id_tabla = i;
                id_entrada = j;                          
            }
        }
    }
    return cargarRespuestaAlgoritmoRemplazo(id_tabla,id_entrada,entrada_vistima);
}


uint64_t now_ms(void){ // mmmmm
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts); // evita cambios de hora del sistema
    return (uint64_t)ts.tv_sec * 1000ull + (uint64_t)ts.tv_nsec / 1000000ull;
}



int gestionarPAGE_FAULT(char* file, char* tag, int nro_pagina){
    log_debug(logger_worker,"(gestionarPAGE_FAULT) - Gestionando PAGE FAULT - File: %s - Tag: %s - Pagina: %d",file,tag,nro_pagina);
    char* mensaje_formateado = string_from_format("LEER %s:%s %d" ,file, tag, nro_pagina); // storage lee toda la pagina y decime que tiene
    
    Mensaje* mensajito = crearMensajito(mensaje_formateado);
    enviarMensajito(mensajito, socket_storage, logger_worker);
    free(mensaje_formateado);
         
    Mensaje* mensaje_recibido = recibirMensajito(socket_storage, logger_worker); // ENUMSERRORSTORAGE CONTENIDO 
    char** datos_recibidos = string_split(mensaje_recibido->mensaje," "); //LIBERAR
    liberarMensajito(mensaje_recibido);
    ErrorStorageEnum cod_op_storage_recv = atoi(datos_recibidos[0]);
    log_debug(logger_worker,"(gestionarPAGE_FAULT) - Codigo de operacion recibido de Storage: %d",cod_op_storage_recv);
    switch (cod_op_storage_recv){
        case OK:
            char* contenido_pagina = datos_recibidos[1]; // no escribimos en memoria
            
            int frame_a_asignar =  devolverFrameLibre(file,tag,nro_pagina);
            escribirPagina(file,tag,nro_pagina,frame_a_asignar, contenido_pagina);
            log_info(logger_worker,"Query %d: Se asigna el Marco: %d a la Página: %d perteneciente al - File: %s - Tag: %s",query->id_query,frame_a_asignar,nro_pagina,file,tag);
            return frame_a_asignar;


        default: 
        error_en_operacion = datos_recibidos[0];
        log_error(logger_worker, "Storage mando cualquier cosa, Cosa que mando: %d", cod_op_storage_recv);
        }
    return -1;
 }


EntradaDeTabla* crearEntradaPagina(int pag_a_asignar, TablaPaginas* tabla){ // tercer parametro cuestionable
    log_debug(logger_worker,"(crearEntradaPagina) - Creando entrada de pagina %d en tabla de paginas File: %s - Tag: %s",pag_a_asignar,tabla->file,tabla->tag);
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
int devolverFrameLibre(char* file, char* tag, int nro_pag){

    // entrada la creamos siempre
    // tenemos que asignarle un marco a la entrada creada para escribir el contenido
    // puede haber marco disponible o no (bitmap)
    // caso feliz: asignamos el frame y escribimos contenido en ese frame
    // caso llorona: elegimos una victima, y vemos si esta modificado
    // caso llorona feliz: no mandamos nada a storage y le asignamos el marco a nuestra entrada y escribimos
    // caso llorona llorona: mandamos write a storage de pag victima y dsp asignamos el marco a nuesta entrada y escribimos
    log_debug(logger_worker,"(gestionarAsignaccionFrame) - Gestionando asignacion de frame para - File: %s - Tag: %s - Pagina: %d",file,tag,nro_pag);   
    int frame = buscarFrameLibreEnBitmap();
    if (frame != -1){
        log_debug(logger_worker,"(gestionarAsignaccionFrame) - Frame libre en bitmap: %d, caso lindo",frame);
        return frame;
    }
    
    log_debug(logger_worker,"(gestionarAsignaccionFrame) - No hubo frame libre, aplicando LA MATANZA");
    frame = aplicarPoliticaReemplazo(file,tag,nro_pag);
    
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

int gestionarBitModificado(RespuestaAlgoritmoReemplazo* resp, char* file, char* tag, int nro_pag){

    if(resp->entrada->bit_modificado == 1){
        escribirEnStorage(resp->entrada); 
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
    log_info(logger_worker, "Query %d: Se libera el Marco: %d perteneciente al - File: %s - Tag: %s",query->id_query,frame_a_retornar,tabla_padre->file,tabla_padre->tag);
    log_info(logger_worker,"Query %d: Se reemplaza la página %s:%s/%d por la %s:%s/%d",query->id_query,tabla_padre->file,tabla_padre->tag,resp->entrada->nro_pag,file,tag,nro_pag);
    free(resp);
    return frame_a_retornar;
}

bool escribirEnStorage(EntradaDeTabla* entrada_a_persistir){
    TablaPaginas* tabla_padre = entrada_a_persistir->tabla;
    char* contenido_a_persistir = 
        string_from_format("ESCRIBIR_BLOQUE %d %s %s %d %s",query->id_query,tabla_padre->file,tabla_padre->tag,entrada_a_persistir->nro_pag,leerBloque(entrada_a_persistir));
    Mensaje* mensajito_a_enviar = crearMensajito(contenido_a_persistir); // ESCRIBIR_BLOQUE query_id file tag nro_pag contenido
    
    enviarMensajito(mensajito_a_enviar,socket_storage,logger_worker);
    free(contenido_a_persistir); 
    Mensaje* respuesta_storage = recibirMensajito(socket_storage,logger_worker); // OK siempre... ponele
    if(respuesta_storage == NULL || !string_equals_ignore_case(respuesta_storage->mensaje ,"OK")){
        liberarMensajito(respuesta_storage);
        log_error(logger_worker, "(escribirEnStorage) Conexion cerrada con storage o no pudo escribir");
        error_en_operacion = respuesta_storage->mensaje;
        return false;
    }
    
    liberarMensajito(respuesta_storage);
    return true;
}

char* leerBloque(EntradaDeTabla* entrada_pagina){ // memoria me desplazo hasta la base del bloque
    TablaPaginas* tabla_padre = entrada_pagina->tabla;

    int desplazamiento_hasta_base = entrada_pagina->nro_frame * tam_pag;
    char* base = memoria + desplazamiento_hasta_base;
    char* leido = malloc(tam_pag + 1);
    if (!leido) {
        log_error(logger_worker,"(leerBloque) - Fallo al reservar memoria para leer bloque");
        return NULL;
    }
    memcpy(leido,base,tam_pag);

    leido[tam_pag] = '\0';
    log_debug(logger_worker,"(leerBloque) - File:Tag %s:%s, Contenido leido:",tabla_padre->file,tabla_padre->tag);
    log_debug(logger_worker,"%s",leido);
    return leido;
}



void hacerRetardo(){
    int retardo_memoria = config_worker->retardo_memoria;
    sleep(retardo_memoria / 1000); // Lo cambiamos despues 
}

bool estaPagEnMemoria(char* file, char* tag, int nro_pag){

    TablaPaginas* tabla_a_consultar = buscarTablaPags(file,tag);
    if(tabla_a_consultar != NULL){
        EntradaDeTabla* entrada = buscarEntradaPorNroPag(tabla_a_consultar->entradas,nro_pag);
        if (entrada!=NULL){
            return (entrada->bit_presencia == 1);        
        }
        
        log_debug(logger_worker, "(estaPagEnMemoria) - No se encontro la entrada en memoria");
        return false;
    }
    
    log_debug(logger_worker,"(estaPagEnMemoria) - No se encontro la tabla en memoria");
    return false;
}

t_list* obtenerEntradasAFlushear(TablaPaginas* tabla_a_flush){
    t_list* entradas_a_retornar = list_create();
    for (int i = 0; i < list_size(tabla_a_flush->entradas); i++){
        EntradaDeTabla* entrada = list_get(tabla_a_flush->entradas,i);
        if (entrada->bit_modificado == 1){
            list_add(entradas_a_retornar,entrada);
        }
    }
    
    return entradas_a_retornar;
}
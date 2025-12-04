#include "bitmap.h"
#include <errno.h>
#include <unistd.h>

void inicializarBitmapYMapeo(){
    
    FILE* arch_bitmap = fopen(RUTA_BITMAP, "r+");
    if (arch_bitmap == NULL) {
        log_error(logger_storage, "ERROR: No se pudo abrir/crear bitmap.bin");
        exit(EXIT_FAILURE);
    }
    
    int arch_bitmap_fd = fileno(arch_bitmap);
    cant_bloques_en_bytes_gb = datos_superblock_gb->cant_bloques / 8;

    log_debug(logger_storage, "DEBUG inicializarBitmapYMapeo: cant_bloques=%d, cant_bloques_en_bytes=%d", 
              datos_superblock_gb->cant_bloques, cant_bloques_en_bytes_gb);
    
    if (cant_bloques_en_bytes_gb <= 0) {
        log_error(logger_storage, "ERROR: cant_bloques_en_bytes_gb es %d", cant_bloques_en_bytes_gb);
        fclose(arch_bitmap);
        exit(EXIT_FAILURE);
    }
    
    // Asegurar que el archivo tiene el tamaño correcto
    if (ftruncate(arch_bitmap_fd, cant_bloques_en_bytes_gb) == -1) {
        log_error(logger_storage, "ERROR: ftruncate falló al redimensionar bitmap.bin");
        fclose(arch_bitmap);
        exit(EXIT_FAILURE);
    }
    
    bitmap_mmap_gb = mmap(
        NULL, cant_bloques_en_bytes_gb, PROT_WRITE | PROT_READ, MAP_SHARED, arch_bitmap_fd, 0);
    
    // Verificar si mmap fue exitoso
    if (bitmap_mmap_gb == MAP_FAILED) {
        log_error(logger_storage, "ERROR: mmap falló al crear bitmap. errno=%d", errno);
        fclose(arch_bitmap);
        exit(EXIT_FAILURE);
    }
    // Crear el bitarray a partir del mapeo
    bitmap_gb = bitarray_create_with_mode(bitmap_mmap_gb, cant_bloques_en_bytes_gb, LSB_FIRST);
    
    if (bitmap_gb == NULL) {
        log_error(logger_storage, "ERROR: bitarray_create_with_mode falló");
        munmap(bitmap_mmap_gb, cant_bloques_en_bytes_gb);
        fclose(arch_bitmap);
        exit(EXIT_FAILURE);
    }
    
    log_debug(logger_storage, "DEBUG: bitmap inicializado correctamente, max_bits=%d", (int)bitarray_get_max_bit(bitmap_gb));
    fclose(arch_bitmap);
}

RespuestaConsultaBitmap* consultarBitmapPorBloquesLibres(int cant_bloques_que_quiero){
    RespuestaConsultaBitmap* response = malloc(sizeof(RespuestaConsultaBitmap));
    char** bloques_a_devolver = string_array_new();

    pthread_mutex_lock(&mutex_bitmap);
    
    for(int i = 0; i < bitarray_get_max_bit(bitmap_gb) && cant_bloques_que_quiero > 0;i++){
        bool esta_libre = bitarray_test_bit(bitmap_gb,i);

        if (esta_libre){
            char* aux = string_new();
            string_append(&aux,string_itoa(i));
            string_array_push(&bloques_a_devolver,aux);
            
            bitarray_set_bit(bitmap_gb,i);
            cant_bloques_que_quiero--;
        }
    }
    
    if(cant_bloques_que_quiero == 0){ 
        response->bloques_encontrados = bloques_a_devolver;
        response->hubo_bloques_libres = true;
    } else{
        log_warning(
        logger_storage,"Cuidadito - (consultarBitmapPorBloquesLibres) - No se encontraron los bloques fisicos necesarios");
    
        limpiarBitsPorStringArray(bloques_a_devolver);
        response->bloques_encontrados = bloques_a_devolver;
        response->hubo_bloques_libres = false;
    }
    
    msync(bitmap_mmap_gb, cant_bloques_en_bytes_gb, MS_SYNC);
    pthread_mutex_unlock(&mutex_bitmap);
    return response;
}


void liberarBitmapYMapeo(){
    munmap(bitmap_mmap_gb, cant_bloques_en_bytes_gb);
    bitarray_destroy(bitmap_gb);
}   

void limpiarBitmap(){
    log_debug(logger_storage, "(limpiarBitmap) - entre");
    
    for (int i = 0; i < bitarray_get_max_bit(bitmap_gb); i++)
    {
        bitarray_clean_bit(bitmap_gb,i);
    }
    
    log_debug(logger_storage,"(limpiarBitmap) - Bitmap reseteado de fabrica");
}
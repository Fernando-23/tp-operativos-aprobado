#include "bitmap.h"

void inicializarBitmapYMapeo(){
    FILE* arch_bitmap = fopen("bitmap.bin", "a+");
    int arch_bitmap_fd = fileno(arch_bitmap);
    cant_bloques_en_bytes_gb = datos_superblock_gb->cant_bloques / 8;
    bitmap_mmap_gb = mmap(
        NULL,cant_bloques_en_bytes_gb,PROT_WRITE | PROT_READ, MAP_SHARED, arch_bitmap_fd, 0);    
    bitmap_gb = bitarray_create_with_mode(bitmap_mmap_gb,cant_bloques_en_bytes_gb,LSB_FIRST);
    
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
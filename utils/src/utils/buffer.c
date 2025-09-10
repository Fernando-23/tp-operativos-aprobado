#include "buffer.h"

// ------------- LEER BUFFER -------------
uint32_t buffer_read_uint32(t_buffer *buffer){
    uint32_t dato_leido;
    memcpy(&dato_leido, buffer->stream, sizeof(uint32_t));
    buffer->stream += sizeof(uint32_t);

    return dato_leido;
}

uint8_t buffer_read_uint8(t_buffer *buffer){
    uint8_t dato_leido;
    memcpy(&dato_leido, buffer->stream, sizeof(uint8_t));
    buffer->stream += sizeof(uint8_t);

    return dato_leido;
}

char *buffer_read_string(t_buffer *buffer, uint32_t tamanio){
    char *string;
    memcpy(&string, buffer->stream, tamanio);   
    buffer->stream += tamanio;

    return string;
}

// ------------- AGREGAR BUFFER -------------

void buffer_add_uint32(t_buffer *buffer, uint32_t data){
    memcpy(buffer->stream +buffer->offset, &data, sizeof(uint32_t));
	buffer->offset+=sizeof(uint32_t);
}

void buffer_add_uint8(t_buffer *buffer, uint8_t data){
    memcpy(buffer->stream +buffer->offset, &data, sizeof(uint8_t));
	buffer->offset+=sizeof(uint8_t);
}

void buffer_add_string(t_buffer *buffer, uint32_t tamanio, char *string){
    memcpy(buffer->stream + buffer->offset, string, tamanio);
    buffer->offset+=tamanio;
}


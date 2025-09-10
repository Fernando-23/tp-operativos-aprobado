#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

typedef struct {
    int tamanio;
    int offset;
    void *stream;
}t_buffer;

void buffer_add_uint32(t_buffer *buffer, uint32_t data);

uint32_t buffer_read_uint32(t_buffer *buffer);

void buffer_add_uint8(t_buffer *buffer, uint8_t data);

uint8_t buffer_read_uint8(t_buffer *buffer);

void buffer_add_string(t_buffer *buffer, uint32_t length, char *string);

char *buffer_read_string(t_buffer *buffer, uint32_t tamanio);

#endif
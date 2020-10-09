#ifndef DOWNGEN_TABLE
#define DOWNGEN_TABLE

#include <stdint.h>


typedef struct {
    uint32_t bitmap;
    uint32_t total_transitions;
} Row;

typedef struct {
    uint32_t row_width;
    uint32_t num_rows;
    Row *rows;
    uint32_t *transitions;
} Table;

typedef struct Image {
    uint32_t width;
    uint32_t height;
    uint8_t *data;
} Image;


Table *table_create(uint32_t width, uint32_t height, char const * const level);
void table_destroy(Table **table);

uint32_t table_bitmap_index(Table *table, uint32_t bitmap);
uint32_t table_next_row(Table *table, uint32_t current_row);

void table_copy_row(Table *table, uint32_t current_row, Image *image);
void table_print(Table *table);

#endif

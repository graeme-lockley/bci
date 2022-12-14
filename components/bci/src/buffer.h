#ifndef BUFFER_H
#define BUFFER_H

typedef struct
{
    char *buffer;
    int32_t item_size;
    int32_t buffer_count;
    int32_t items_count;
} Buffer;

extern Buffer *buffer_new(int item_size);
    
extern void buffer_free(Buffer *buffer);
extern void *buffer_free_use(Buffer *buffer);
extern void *buffer_content(Buffer *buffer);
extern int32_t buffer_count(Buffer *buffer);
extern int32_t buffer_offset(Buffer *buffer);
extern void buffer_append(Buffer *b, void *v, int count);
extern void buffer_write(Buffer *b, int32_t offset, void *v, int count);

#endif

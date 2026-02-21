#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "buffer.h"


void initBuffer(Buffer* buf, int size) {
    buf->data  = malloc(size);
    buf->cap   = size;
    buf->start = 0;
    buf->end   = 0;
}

void clear(Buffer* buf) {
    free(buf->data);
    buf->data  = NULL;
    buf->cap   = 0;
    buf->start = 0;
    buf->end   = 0;
}

int bufferSize(Buffer* buf) {
    return buf->end - buf->start;
}

void append(Buffer* buf, const char* s) {
    int size = strlen(s);

    if (buf->end + size > buf->cap) {
        printf("CALLED MALLOC!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        int currentSize = buf->end - buf->start;
        int newCap = currentSize + size + 32;
        char* tmp = malloc(newCap);

        if (currentSize > 0)
            memcpy(tmp, buf->data + buf->start, currentSize);

        free(buf->data);
        buf->data  = tmp;
        buf->cap   = newCap;
        buf->start = 0;
        buf->end   = currentSize;
    }

    memcpy(buf->data + buf->end, s, size);
    buf->end += size;
    buf->data[buf->end] = '\0';
}

void consume(Buffer* buf, int n) {
    if (n >= bufferSize(buf)) {
        buf->start = 0;
        buf->end   = 0;
    } else {
        buf->start += n;
    }
}

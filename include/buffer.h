#ifndef BUFFER_H
#define BUFFER_H

typedef struct s_buffer {
    char*   data;
    int     cap;
    int     start;
    int     end;
} Buffer;

void    initBuffer(Buffer* buf, int size);
void    append(Buffer* buf, const char* s);
void    consume(Buffer* buf, int n);
void    clear(Buffer* buf);
int     bufferSize(Buffer* buf);


#endif

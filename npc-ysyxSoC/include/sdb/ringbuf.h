#ifndef __RINGBUF_H__
#define __RINGBUF_H__

#define RINGBUF_SIZE 10
#define MAX_STRING_LEN 128

typedef struct {
    char buf[RINGBUF_SIZE][MAX_STRING_LEN];
    int head;
    int tail;
		int count;
} RingBuffer;

extern RingBuffer inst_ringbuf;

void init_ringbuf(RingBuffer *rb);


void ringbuf_put(RingBuffer *rb, char *data);


void ringbuf_print(RingBuffer *rb);


int ringbuf_count(RingBuffer *rb);


void ringbuf_clear(RingBuffer *rb);

#endif


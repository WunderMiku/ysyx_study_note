#include "ringbuf.h"

#include <assert.h>
#include <string.h>


RingBuffer inst_ringbuf;

// 初始化环形缓冲区
void init_ringbuf(RingBuffer *rb) {
	assert(rb != NULL);
	rb->head = rb->tail = rb->count = 0;
}

// 向环形缓冲区(的head)写入数据
void ringbuf_put(RingBuffer *rb, char *data) {
	assert(rb!= NULL);
	assert(data != NULL);

	strncpy(rb->buf[rb->head], data, MAX_STRING_LEN);
	rb->buf[rb->head][MAX_STRING_LEN - 1] = '\0';

	rb->head = (rb->head + 1) % RINGBUF_SIZE;
	if(!ringbuf_full(rb)) {
		rb->count++;
	} else {
		rb->tail = (rb->tail + 1) % RINGBUF_SIZE;
	}

	// Log("RingBuffer:\n\t Head: %s\n\t tail: %s\n\t count: %d", rb->buf[rb->head - 1], rb->buf[rb->tail], rb->count);
}

// 从环形缓冲区输出全部数据
void ringbuf_print(RingBuffer *rb) {
	assert(rb != NULL);
	printf("===== RingBuffer: %d inst(s) =====\n", rb->count);
	for(int i = 0; i < rb->count; i++) {
		if(i != rb->count - 1) {
			printf("%s\n", rb->buf[(rb->tail + i) % RINGBUF_SIZE]);
		} else {
			printf(ANSI_FG_RED "%s\n" ANSI_NONE, rb->buf[(rb->tail + i) % RINGBUF_SIZE]);
		}
	}
	printf("              ^\n");
	printf("              |\n");
	printf(" Last_inst ---+\n");
	printf("\n");
}

// 检查环形缓冲区是否为空
bool ringbuf_empty(RingBuffer *rb) {
	assert(rb != NULL);
	return rb->count == 0;
}

// 检查环形缓冲区是否已满
bool ringbuf_full(RingBuffer *rb) {
	assert(rb != NULL);
	return rb->count == RINGBUF_SIZE;
}

// 获取当前元素数量
int ringbuf_count(RingBuffer *rb) {
	assert(rb != NULL);
	return rb->count;
}

// 清空环形缓冲区
void ringbuf_clear(RingBuffer *rb) {
	assert(rb != NULL);
	for(int i = 0; i < RINGBUF_SIZE; i++) {
		rb->buf[i][0] = '\0';
	}
}
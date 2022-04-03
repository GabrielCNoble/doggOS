#ifndef K_IO_H
#define K_IO_H

#include <stdint.h>
#include "rt/atm.h"

enum K_IO_STREAM_FLAGS
{
    // K_IO_STREAM_FLAG_BUFFER_DATA = 1,
    K_IO_STREAM_FLAG_READ = 1 << 1,
    K_IO_STREAM_FLAG_WRITE = 1 << 2,
    K_IO_STREAM_FLAG_BLOCKED = 1 << 3,
    K_IO_STREAM_FLAG_CLEAR_READ = 1 << 4
};

#define K_IO_STREAM_BUF_DATA_SIZE (4096 - (sizeof(struct k_io_stream_buf_t *) * 2))

struct k_io_stream_buf_t
{
    struct k_io_stream_buf_t *next;
    struct k_io_stream_buf_t *prev;
    uint8_t data[K_IO_STREAM_BUF_DATA_SIZE];
};

struct k_io_stream_t
{
    struct k_io_stream_t *next;
    uint32_t (*request_read)(struct k_io_stream_t *stream, uint32_t offset, uint32_t size);
    uint32_t (*request_write)(struct k_io_stream_t *stream, uint32_t offset, uint32_t size);
    // uint32_t (*update)(struct k_io_stream_t *stream);

    void *target;
    uint32_t read_offset;
    uint32_t write_offset;
    uint32_t available_count;
    // uint32_t free_size;
    // uint32_t alloc_size;
    uint32_t flags;
    k_rt_cond_t condition;
    /*
        FIXME: a doubly linked list is fast for size increases but will become
        very slow during seeks in case the stream buffer grows too much. 

        An alternative would be a balanced binary tree. Insertion would be a bit
        slower, but seeking would be faster.
    */
    struct k_io_stream_buf_t *buffers;
    struct k_io_stream_buf_t *last_buffer;
    struct k_io_stream_buf_t *read_buffer;
    struct k_io_stream_buf_t *write_buffer;
    struct k_io_stream_buf_t *free_buffers;
};

struct k_io_stream_t *k_io_AllocStream();

void k_io_FreeStream(struct k_io_stream_t *stream);

void k_io_AllocStreamBuffer(struct k_io_stream_t *stream);

uint32_t k_io_FlushStream(struct k_io_stream_t *stream);

uint32_t k_io_SeekStream(struct k_io_stream_t *stream, uint32_t offset, uint32_t pos);

uint32_t k_io_ReadStreamData(struct k_io_stream_t *stream, void *data, uint32_t size);

uint32_t k_io_ReadStream(struct k_io_stream_t *stream, uint32_t offset, void *data, uint32_t size);

uint32_t k_io_WriteStreamData(struct k_io_stream_t *stream, void *data, uint32_t size);

uint32_t k_io_WriteStream(struct k_io_stream_t *stream, uint32_t offset, void *data, uint32_t size);

uint32_t k_io_UnblockStream(struct k_io_stream_t *stream);

uint32_t k_io_BlockStream(struct k_io_stream_t *stream);

uint32_t k_io_SignalStream(struct k_io_stream_t *stream);

uint32_t k_io_UnsignalStream(struct k_io_stream_t *stream);

uint32_t k_io_WaitStream(struct k_io_stream_t *stream);

#endif
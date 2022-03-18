#include "io.h"
#include "rt/alloc.h"
#include "rt/mem.h"
#include "proc/proc.h"

struct k_io_stream_t *k_io_AllocStream(uint32_t flags)
{
    struct k_io_stream_t *stream = k_rt_Malloc(sizeof(struct k_io_stream_t), 4);
    stream->buffers = NULL;
    stream->read_buffer = NULL;
    stream->write_buffer = NULL;
    stream->last_buffer = NULL;
    stream->read_offset = 0;
    stream->write_offset = 0;
    stream->alloc_size = 0;
    stream->free_size = 0;
    stream->free_buffers = NULL;
    stream->flags = flags & (K_IO_STREAM_FLAG_READ | K_IO_STREAM_FLAG_WRITE);
    // struct k_proc_process_t *current_process = k_proc_GetCurrentProcess();
    // stream->next = current_process->streams;
    // current_process->streams = stream;
    return stream;
}

void k_io_FreeStream(struct k_io_stream_t *stream)
{
    if(stream)
    {
        struct k_io_stream_buf_t *buffer = stream->buffers;

        while(buffer)
        {
            struct k_io_stream_buf_t *next_buffer = buffer->next;
            k_rt_Free(buffer);
            buffer = next_buffer;
        }

        k_rt_Free(stream);
    }
}

void k_io_AllocStreamBuffer(struct k_io_stream_t *stream)
{
    if(stream)
    {
        struct k_io_stream_buf_t *buffer;

        if(stream->free_buffers)
        {
            do
            {
                buffer = stream->free_buffers;
            }
            while(!k_rt_CmpXcgh((uintptr_t *)&stream->free_buffers, (uintptr_t)buffer, (uintptr_t)buffer->next, NULL));
        }
        else
        {
            buffer = k_rt_Malloc(sizeof(struct k_io_stream_buf_t ), 4);
        }
        

        if(!stream->buffers)
        {
            stream->buffers = buffer;
        }
        else
        {
            stream->last_buffer->next = buffer;
        }

        buffer->prev = stream->last_buffer;
        buffer->next = stream->buffers;
        stream->last_buffer = buffer;
        stream->alloc_size += K_IO_STREAM_BUF_DATA_SIZE;
        stream->free_size += K_IO_STREAM_BUF_DATA_SIZE;

        // k_sys_TerminalPrintf("alloc buf %x\n", buffer);
    }
}

uint32_t k_io_FlushStream(struct k_io_stream_t *stream)
{
    
}

uint32_t k_io_SeekStream(struct k_io_stream_t *stream, uint32_t offset, uint32_t pos)
{

}

uint32_t k_io_ReadStream(struct k_io_stream_t *stream, void *data, uint32_t size)
{
    if(!stream)
    {
        return K_STATUS_INVALID_STREAM;
    }

    if(stream->flags & K_IO_STREAM_FLAG_BLOCKED)
    {
        return K_STATUS_BLOCKED_STREAM;
    }

    if(!data || !size)
    {
        return K_STATUS_INVALID_DATA;
    }

    if(!stream->buffers || stream->read_offset == stream->write_offset)
    {
        return K_STATUS_EMPTY_STREAM;
    }

    if(!stream->read_buffer)
    {
        stream->read_buffer = stream->buffers;
    }

    uint32_t data_size = stream->alloc_size - stream->free_size;

    uint32_t data_offset = 0;
    uint32_t freed_size = 0;
    struct k_io_stream_buf_t *last_freed_buffers = NULL;
    struct k_io_stream_buf_t *first_freed_buffer = stream->read_buffer;

    if(size > data_size)
    {
        size = data_size;
    }

    while(size)
    {
        uint32_t buffer_offset = stream->read_offset % K_IO_STREAM_BUF_DATA_SIZE;
        uint32_t copy_size = K_IO_STREAM_BUF_DATA_SIZE - buffer_offset;
        struct k_io_stream_buf_t *buffer = stream->read_buffer;

        if(copy_size <= size)
        {
            last_freed_buffers = buffer;
            stream->read_buffer = stream->read_buffer->next;
            freed_size += K_IO_STREAM_BUF_DATA_SIZE;
        }
        else
        {
            copy_size = size;
        }

        k_rt_CopyBytes((uint8_t *)data + data_offset, buffer->data + buffer_offset, copy_size);
        data_offset += copy_size;
        stream->read_offset = (stream->read_offset + copy_size) % stream->alloc_size;
        size -= copy_size;
    }

    if(stream->flags & K_IO_STREAM_FLAG_CLEAR_READ && last_freed_buffers)
    {
        last_freed_buffers->next = stream->free_buffers;
        stream->free_buffers = first_freed_buffer;
        stream->free_size += freed_size;
    }
    
    return K_STATUS_OK;
}

uint32_t k_io_WriteStream(struct k_io_stream_t *stream, void *data, uint32_t size)
{
    if(!stream)
    {
        return K_STATUS_INVALID_STREAM;
    }

    if(!data || !size)
    {
        return K_STATUS_INVALID_DATA;
    }

    if(stream->flags & K_IO_STREAM_FLAG_BLOCKED)
    {
        return K_STATUS_BLOCKED_STREAM;
    }

    while(stream->free_size < size)
    {
        k_io_AllocStreamBuffer(stream);
    }

    if(!stream->write_buffer)
    {
        stream->write_buffer = stream->buffers;
    }

    uint32_t data_offset = 0;

    stream->free_size -= size;

    while(size)
    {
        uint32_t buffer_offset = stream->write_offset % K_IO_STREAM_BUF_DATA_SIZE;
        uint32_t copy_size = K_IO_STREAM_BUF_DATA_SIZE - buffer_offset;
        struct k_io_stream_buf_t *buffer = stream->write_buffer;

        if(copy_size <= size)
        {
            stream->write_buffer = stream->write_buffer->next;
        }
        else
        {
            copy_size = size;
        }

        k_rt_CopyBytes(&buffer->data[buffer_offset], ((uint8_t *)data) + data_offset, copy_size);
        data_offset += copy_size;
        stream->write_offset = (stream->write_offset + copy_size) % stream->alloc_size;
        size -= copy_size;
    }

    return K_STATUS_OK;
}

uint32_t k_io_UnblockStream(struct k_io_stream_t *stream)
{
    if(stream)
    {
        stream->flags &= ~K_IO_STREAM_FLAG_BLOCKED;
    }
}

uint32_t k_io_BlockStream(struct k_io_stream_t *stream)
{
    if(stream)
    {
        stream->flags |= K_IO_STREAM_FLAG_BLOCKED;
    }
}
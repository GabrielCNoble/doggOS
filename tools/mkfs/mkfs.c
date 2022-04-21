#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mkfs.h"

// #include "fs/defs.h"
#include "fs/pup.h"

enum ARGS
{
    ARG_BLOCK_COUNT,
    ARG_BLOCK_SIZE, 
    ARG_FILE,
    ARG_LAST,
};

char *arg_strs[] = 
{
    [ARG_BLOCK_COUNT] = "block_count",
    [ARG_BLOCK_SIZE] = "block_size",
    [ARG_FILE] = "file",
};

struct file_arg_t
{
    struct file_arg_t *next;
    char *path;
};

struct k_fs_pup_range_t find_free_block_range(uint8_t *disk_buffer, uint32_t count, uint32_t usage)
{
    struct k_fs_pup_root_t *root = (struct k_fs_pup_root_t *)disk_buffer;
    struct k_fs_pup_range_t range = {};
    uint32_t first_block = 0;
    uint32_t block_count = 0;
    uint8_t status_mask = 0;
    uint8_t alloc_mask = 0;
    
    switch(usage)
    {
        case K_FS_PUP_BLOCK_STATUS_DATA:
            status_mask = K_FS_PUP_BLOCK_STATUS_FREE;
        break;
        
        case K_FS_PUP_BLOCK_STATUS_NODE:
            status_mask = K_FS_PUP_BLOCK_STATUS_NODE;
        break;
    }
    
    for(uint32_t block_index = 0; block_index < root->bitmask_block_count; block_index++)
    {
        uint8_t *bitmask_block = disk_buffer + (root->bitmask_block_start + block_index) * root->block_size;
        
        for(uint32_t byte_index = 0; byte_index < root->block_size; byte_index++)
        {    
            for(uint32_t pair_shift = 0; pair_shift < 8; pair_shift += K_FS_PUP_FREE_BITMASK_BLOCK_BITS)
            {
                uint8_t bitmask_byte = (bitmask_block[byte_index] >> pair_shift) & 0x3;
                
                if(bitmask_byte == K_FS_PUP_BLOCK_STATUS_FREE || bitmask_byte == status_mask)
                {
                    if(!first_block)
                    {
                        first_block = root->available_block_start;
                        first_block += root->block_size * K_FS_PUP_FREE_BITMASK_BLOCKS_PER_BYTE * block_index;
                        first_block += byte_index * K_FS_PUP_FREE_BITMASK_BLOCKS_PER_BYTE;
                        // range.first_block += (root->block_size * block_index + byte_index) * K_FS_PUP_FREE_BITMASK_BLOCKS_PER_BYTE;
                        first_block += pair_shift >> 1;
                        block_count = 0;
                    }
                    
                    block_count++;
                    
                    if(block_count == count)
                    {
                        // printf("return range %lld, %lld\n", range.first_block, range.block_count);
                        range.first_count = K_FS_PUP_RANGE_FIRST_COUNT(first_block, block_count);
                        return range;
                    }
                }
                
                first_block = 0;
            }
        }
    }
    
    return (struct k_fs_pup_range_t){};
}

void set_range_status(uint8_t *disk_buffer, struct k_fs_pup_range_t *range, uint32_t status)
{
    struct k_fs_pup_root_t *root = (struct k_fs_pup_root_t *)disk_buffer;
    uint32_t range_first_block = K_FS_PUP_RANGE_FIRST_BLOCK(range->first_count);
    uint32_t range_block_count = K_FS_PUP_RANGE_BLOCK_COUNT(range->first_count);
    if(range_first_block)
    {
        uint32_t blocks_per_bitmask_block = root->block_size * K_FS_PUP_FREE_BITMASK_BLOCKS_PER_BYTE;
        uint32_t first_block = range_first_block - root->available_block_start;
        uint32_t last_block = range_first_block + range_block_count;
        
        for(uint32_t block_index = first_block; block_index < last_block; block_index++)
        {
            uint32_t bitmask_block_index = block_index / blocks_per_bitmask_block;
            uint32_t bitmask_byte_index = (block_index % blocks_per_bitmask_block) >> K_FS_PUP_FREE_BITMASK_BLOCK_BITS;
            uint8_t *block_bitmask = disk_buffer + (root->bitmask_block_start + bitmask_block_index) * root->block_size;
            uint32_t status_shift = K_FS_PUP_FREE_BITMASK_BLOCK_BITS * (block_index & 0x3);
            block_bitmask[bitmask_byte_index] &= ~(K_FS_PUP_BLOCK_STATUS_FULL << status_shift);
            block_bitmask[bitmask_byte_index] |= status << status_shift;
        }
    }
}

struct k_fs_pup_range_t alloc_blocks(uint8_t *disk_buffer, uint32_t count)
{
    struct k_fs_pup_root_t *root = (struct k_fs_pup_root_t *)disk_buffer;
    // printf("alloc_blocks\n");
    struct k_fs_pup_range_t range = find_free_block_range(disk_buffer, count, K_FS_PUP_BLOCK_STATUS_DATA);
    uint32_t first_block = K_FS_PUP_RANGE_FIRST_BLOCK(range.first_count);
    if(first_block)
    {
        set_range_status(disk_buffer, &range, K_FS_PUP_BLOCK_STATUS_DATA);
    }
    
    return range;
}

struct k_fs_pup_link_t alloc_node(uint8_t *disk_buffer, uint32_t type)
{
    struct k_fs_pup_root_t *root = (struct k_fs_pup_root_t *)disk_buffer;
    struct k_fs_pup_link_t link = {};
    // printf("alloc_node\n");
    struct k_fs_pup_range_t range = find_free_block_range(disk_buffer, 1, K_FS_PUP_BLOCK_STATUS_NODE);
    uint32_t first_block = K_FS_PUP_RANGE_FIRST_BLOCK(range.first_count);
    if(first_block)
    {
        set_range_status(disk_buffer, &range, K_FS_PUP_BLOCK_STATUS_NODE);
        
        struct k_fs_pup_node_t *node_block = (struct k_fs_pup_node_t *)(disk_buffer + first_block * root->block_size);
        uint32_t node_count = root->block_size / sizeof(struct k_fs_pup_node_t);

        for(uint32_t node_index = 0; node_index < node_count; node_index++)
        {    
            if(node_block[node_index].type == K_FS_PUP_NODE_TYPE_NONE)
            {
                node_block[node_index].type = type;
                node_block[node_index].flags = 0;
                
                // printf("block index: %d\n", range.first_block);
                // printf("node index: %d\n", node_index);
                // struct k_fs_pup_node_t *node = node_block + node_index;
                // node->type = type;
                // node->flags = 0;
                // printf("node %p, type %d, %d\n", node, node->type, type);
                link.link = node_index;
                link.link |= first_block << root->node_index_shift;
                // printf("alloc node %llx\n", link.link);
                // printf("%d\n", root->node_index_shift);
                break;
            }
        }
    }
    
    return link;
}

struct k_fs_pup_node_t *get_node(uint8_t *disk_buffer, struct k_fs_pup_link_t node_address)
{
    struct k_fs_pup_root_t *root = (struct k_fs_pup_root_t *)disk_buffer;
    struct k_fs_pup_node_t *node = NULL;
    
    if(node_address.link)
    {
        uint64_t node_block_index = node_address.link >> root->node_index_shift;
        uint64_t node_index_mask = (1 << root->node_index_shift) - 1;
        uint64_t node_index = node_address.link & node_index_mask;
        // printf("block index: %d\n", node_block_index);
        // printf("node index: %d\n", node_index);
        struct k_fs_pup_node_t *node_block = (struct k_fs_pup_node_t *)(disk_buffer + node_block_index * root->block_size);
        node = node_block + node_index;
    }
    return node;
}

void add_node(uint8_t *disk_buffer, char *parent_path, char *node_name)
{
    struct k_fs_pup_root_t *root = (struct k_fs_pup_root_t *)disk_buffer;
    struct k_fs_pup_node_t *parent_node = find_node(disk_buffer, parent_path, NULL);
    // printf("parent node: %p, %d\n", parent_node, parent_node->type);
    if(parent_node && parent_node->type == K_FS_PUP_NODE_TYPE_DIR)
    {
        // printf("found parent %s (%p)\n", parent_path, parent_node);
        struct k_fs_pup_node_t *child_node = find_node(disk_buffer, node_name, parent_node);
        
        if(!child_node)
        {
            struct k_fs_pup_link_t node_link = alloc_node(disk_buffer, K_FS_PUP_NODE_TYPE_DIR);
            
            for(uint32_t range_index = 0; range_index < K_FS_PUP_MAX_RNODE_RANGES; range_index++)
            {
                uint32_t first_block = K_FS_PUP_RANGE_FIRST_BLOCK(parent_node->ranges[range_index].first_count);
                struct k_fs_pup_dirlist_t *entry_list = (struct k_fs_pup_dirlist_t *)(disk_buffer + first_block * root->block_size);
                struct k_fs_pup_range_t *range = parent_node->ranges + range_index;
                uint32_t range_block_count = K_FS_PUP_RANGE_BLOCK_COUNT(range->first_count);
                uint32_t max_entries = (range_block_count * root->block_size - sizeof(uint32_t)) / sizeof(struct k_fs_pup_dirent_t);
                // uint32_t max_entries = (parent_node->ranges[range_index].block_count * root->block_size - sizeof(uint32_t)) / sizeof(struct k_fs_pup_dirent_t);
                
                if(entry_list->used_count < max_entries)
                {
                    struct k_fs_pup_dirent_t *entry = entry_list->entries + entry_list->used_count;
                    
                    strcpy(entry->name, node_name);
                    entry->node = node_link;
                    // printf("node %lld (%s) added to block %p, at index %d\n", node_link.link, node_name, entry_list, entry_list->used_count);
                    
                    entry_list->used_count++;
                    
                    struct k_fs_pup_node_t *node = get_node(disk_buffer, node_link);
                    node->ranges[0] = alloc_blocks(disk_buffer, 1);
                    break;
                }
            }
        }
    }
}

void free_node(uint8_t *disk_buffer, struct k_fs_pup_link_t link)
{
    // struct k_fs_pup_root_t *root = (struct k_fs_pup_root_t *)disk_buffer;
    // uint64_t node_block_index = node_address >> root->node_index_shift;
    // uint64_t node_index_mask = ~((1 << root->node_index_shift) - 1);
    // uint64_t node_index = node_address & node_index_mask;
    // struct k_fs_pup_node_t *node_block = (struct k_fs_pup_node_t *)(disk_buffer + (root->available_block_start + node_block_index) * root->block_size);
    // struct k_fs_pup_node_t *node = node_block + node_index;
    // node->type = K_FS_PUP_NODE_TYPE_NONE;
    
    // uint8_t *bitmask_block = disk_buffer + (root->bitmask_block_start + node_block_index)
}

struct k_fs_pup_node_t *find_node(uint8_t *disk_buffer, char *path, struct k_fs_pup_node_t *start_node)
{
    struct k_fs_pup_root_t *root = (struct k_fs_pup_root_t *)disk_buffer;
    struct k_fs_pup_node_t *node = NULL;
    uint32_t path_cursor = 0;
    
    char path_fragment[512];
    uint32_t path_fragment_cursor = 0;
    
    // while(path[path_cursor] == ' ')
    // {
    //     path_cursor++;
    // }
    
    if(!start_node && path[path_cursor] == '/')
    {
        path_cursor++;
        node = get_node(disk_buffer, root->root_node);
    }
    else
    {
        node = start_node;
    }
        
    while(path[path_cursor] && node)
    {
        path_fragment_cursor = 0;
        while(path[path_cursor] != '/' && path[path_cursor])
        {
            path_fragment[path_fragment_cursor] = path[path_cursor];
            path_fragment_cursor++;
            path_cursor++;
        }
        
        if(path[path_cursor] == '/')
        {
            path_cursor++;
        }
        
        path_fragment[path_fragment_cursor] = '\0';
        struct k_fs_pup_node_t *next_node = NULL; 
        
        // printf("node type: %d\n", node->type);
        
        if(node->type == K_FS_PUP_NODE_TYPE_DIR)
        {                
            // printf("%s\n", path_fragment);
            
            for(uint32_t range_index = 0; range_index < K_FS_PUP_MAX_RNODE_RANGES && !next_node; range_index++)
            {
                struct k_fs_pup_range_t *range = node->ranges + range_index;
                uint32_t first_block = K_FS_PUP_RANGE_FIRST_BLOCK(range->first_count);
                struct k_fs_pup_dirlist_t *entry_list = (struct k_fs_pup_dirlist_t *)(disk_buffer + first_block * root->block_size);
                // uint32_t entry_count = range->block_count * (root->block_size / sizeof(struct k_fs_pup_direntry_t));
                // printf("blah\n");
                for(uint32_t entry_index = 0; entry_index < entry_list->used_count; entry_index++)
                {
                    struct k_fs_pup_dirent_t *entry = entry_list->entries + entry_index;
                    // printf("%s\n", entry->name);
                    
                    if(!entry->node.link)
                    {
                        break;
                    }
                    
                    if(!strcmp(path_fragment, entry->name))
                    {
                        // printf("%s\n", entry->name);
                        next_node = get_node(disk_buffer, entry->node);
                        // printf("%p - %s\n", next_node, entry->name);
                        break;
                    }
                }
            }
        }
        
        node = next_node;
        // printf("%p\n", node);
    }
    // }
    
    return node;
}

void print_fs_recursive(uint8_t *disk_buffer, struct k_fs_pup_node_t *parent_node, uint32_t depth)
{
    struct k_fs_pup_root_t *root = (struct k_fs_pup_root_t *)disk_buffer;
    uint32_t blank_line = 0;
    if(parent_node->type == K_FS_PUP_NODE_TYPE_DIR)
    {
        for(uint32_t range_index = 0; range_index < K_FS_PUP_MAX_RNODE_RANGES; range_index++)
        {
            struct k_fs_pup_range_t *range = parent_node->ranges + range_index;
            uint32_t first_block = K_FS_PUP_RANGE_FIRST_BLOCK(range->first_count);
            if(first_block)
            {
                struct k_fs_pup_dirlist_t *entry_list = (struct k_fs_pup_dirlist_t *)(disk_buffer + first_block * root->block_size);
                for(uint32_t entry_index = 0; entry_index < entry_list->used_count; entry_index++)
                {
                    struct k_fs_pup_dirent_t *entry = entry_list->entries + entry_index; 
                    
                    // if(depth)
                    // {
                    // 
                    // }
                    for(uint32_t depth_index = 0; depth_index < depth; depth_index++)
                    {
                        putchar(' ');
                    }
                    
                    putchar('|');
                    putchar('-');
                    
                    printf("%s\n", entry->name);
                    struct k_fs_pup_node_t *child_node = get_node(disk_buffer, entry->node);
                    // if(child_node)
                    // {
                    // 
                    // }
                    
                    print_fs_recursive(disk_buffer, child_node, depth + 2);
                }
            }
        }
        
        // printf("\n");
    }
    
    // printf("\n");
}

void print_fs(uint8_t *disk_buffer)
{
    struct k_fs_pup_root_t *root = (struct k_fs_pup_root_t *)disk_buffer;
    struct k_fs_pup_node_t *root_node = get_node(disk_buffer, root->root_node);
    
    print_fs_recursive(disk_buffer, root_node, 0);
}

int main(int argc, char *argv[])
{
    if(argc > 1)
    {
        uint32_t block_count = 0;
        uint32_t block_size = 512;
        struct file_arg_t *files = NULL;
        struct file_arg_t *last_file = NULL;
        
        for(uint32_t arg_index = 1; arg_index < argc; arg_index++)
        {
            char *arg_str = argv[arg_index];
            uint32_t arg_str_cursor = 0;
            uint32_t index = 0;
            for(; index < ARG_LAST; index++)
            {
                if(strstr(arg_str, arg_strs[index]) == arg_str)
                {
                    arg_str_cursor += strlen(arg_strs[index]);
                    
                    while(arg_str[arg_str_cursor] == ' ')
                    {
                        arg_str_cursor++;
                    }
                    
                    if(arg_str[arg_str_cursor] != '=')
                    {
                        printf("ERROR: missing value for arg at index %d!", arg_index);
                        return -1;
                    }
                    
                    arg_str_cursor++;
                    
                    while(arg_str[arg_str_cursor] == ' ')
                    {
                        arg_str_cursor++;
                    }
                    
                    if(!arg_str[arg_str_cursor])
                    {
                        printf("ERROR: missing value for arg at index %d!\n", arg_index);
                        return -1;
                    }
                    
                    switch(index)
                    {
                        case ARG_BLOCK_COUNT:
                            block_count = atoi(arg_str + arg_str_cursor);
                            if(!block_count)
                            {
                                printf("ERROR: invalid block count!\n");
                                return -1;
                            }
                        break;
                        
                        case ARG_BLOCK_SIZE:
                            block_size = atoi(arg_str + arg_str_cursor);
                            if(!block_size)
                            {
                                printf("ERROR: invalid block size!\n");
                                return -1;
                            }
                        break;
                        
                        case ARG_FILE:
                        {
                            struct file_arg_t *file = calloc(sizeof(struct file_arg_t), 1);
                            file->path = arg_str + arg_str_cursor;
                            
                            if(!files)
                            {
                                files = file;
                            }
                            else
                            {
                                last_file->next = file;
                            }
                            last_file = file;
                        }
                        break;
                    }
                    
                    break;
                }
            }
            
            if(index == ARG_LAST)
            {
                printf("ERROR: unknown arg at index %d!\n", arg_index);
                return -1;
            }
        }
        
        if(!block_count)
        {
            printf("ERROR: no block count provided!\n");
            return -1;
        }
        
        if(block_size < 512)
        {
            printf("ERROR: block size should be at least 512 bytes!\n");
            return -1;
        }
        
        for(uint32_t test_mask = 0x80000000; test_mask; test_mask >>= 1)
        {
            if((block_size & test_mask) && (block_size & (test_mask - 1)))
            {
                printf("ERROR: block size is not a power of two!\n");
                return -1;
            }
        }
        
        uint8_t *disk_buffer = calloc(1, block_size * block_count);
        // memset(disk_buffer, 0, block_size * block_count);
        uint32_t covered_block_count = block_size * 4 + 1;
        
        uint32_t bitmask_block_count = block_count / covered_block_count;
        
        if(block_count % covered_block_count)
        {
            bitmask_block_count++;
        }
        
        struct k_fs_pup_root_t *root = (struct k_fs_pup_root_t *)disk_buffer;
        strcpy(root->ident, K_FS_PUP_MAGIC);
        root->block_size = block_size;
        root->bitmask_block_start = 1;
        root->bitmask_block_count = bitmask_block_count;
        root->available_block_start = root->bitmask_block_start + bitmask_block_count;
        root->node_index_shift = root->block_size / sizeof(struct k_fs_pup_node_t);
        
        uint16_t mask = 0x8000;
        for(uint16_t index = 15; index; index--)
        {
            if((root->node_index_shift & mask) && (root->node_index_shift & (mask - 1)))
            {
                root->node_index_shift = index + 1;
                break;
            }
            
            mask >>= 1;
        }
        
        root->root_node = alloc_node(disk_buffer, K_FS_PUP_NODE_TYPE_DIR);
        struct k_fs_pup_node_t *root_node = get_node(disk_buffer, root->root_node);
        root_node->ranges[0] = alloc_blocks(disk_buffer, 1);
        
        // struct k_fs_pup_dirlist_t *entry_list = (struct k_fs_pup_dirlist_t *)(disk_buffer + root_node->ranges[0].first_block * root->block_size);
        // printf("%d\n", entry_list->used_count);
        
        // struct k_fs_pup_dirlist_t *entry_list = (struct k_fs_pup_dirlist_t *)(disk_buffer + root_node->ranges[0].first_block * root->block_size);
        // entry_list->used_count = 0;
        
        
        // struct k_fs_pup_range_t range;
        // alloc_blocks(disk_buffer, 1);
        // alloc_blocks(disk_buffer, 1);
        // alloc_blocks(disk_buffer, 1);
        // alloc_blocks(disk_buffer, 1);
        // alloc_blocks(disk_buffer, 1);
        // alloc_blocks(disk_buffer, 1);
        // alloc_blocks(disk_buffer, 1);
        
        
        // add_node(disk_buffer, "/", "fokn___");
        // add_node(disk_buffer, "/", "shitn__");
        // add_node(disk_buffer, "/", "pissin_");
    
        struct k_fs_pup_node_t *node;
        
        // node = find_node(disk_buffer, "/fokn___", NULL);
        // printf("node %p, type %d\n", node, node->type);
        // 
        // node = find_node(disk_buffer, "/shitn__", NULL);
        // printf("node %p, type %d\n", node, node->type);
        // 
        // node = find_node(disk_buffer, "/pissin_", NULL);
        // printf("node %p, type %d\n", node, node->type);
         
        // add_node(disk_buffer, "/fokn___", "crapn____");
        // add_node(disk_buffer, "/fokn___", "bitchn______");
        // node = find_node(disk_buffer, "/fokn___/crapn____", NULL);
        // printf("node %p, type %d\n", node, node->type);
        
        // node = find_node(disk_buffer, "/fokn___/bitchn______", NULL);
        // printf("node %p, type %d\n", node, node->type);
    
        add_node(disk_buffer, "/", "a");
        add_node(disk_buffer, "/", "b");
        add_node(disk_buffer, "/", "c");
        add_node(disk_buffer, "/", "d");
        // 
        add_node(disk_buffer, "/a", "e");
        add_node(disk_buffer, "/a", "f");
        add_node(disk_buffer, "/a", "g");
        // 
        add_node(disk_buffer, "/a/e", "h");
        add_node(disk_buffer, "/a/e", "i");
        // 
        add_node(disk_buffer, "/a/e/f", "j"); 
        add_node(disk_buffer, "/", "cock");
        add_node(disk_buffer, "/cock", "ass");
        add_node(disk_buffer, "/cock/ass", "shit0");
        add_node(disk_buffer, "/cock/ass", "shit1");
        add_node(disk_buffer, "/cock/ass", "shit2");
        add_node(disk_buffer, "/cock/ass", "shit3");
        add_node(disk_buffer, "/cock/ass", "shit4");
        add_node(disk_buffer, "/cock/ass", "shit5");
        add_node(disk_buffer, "/cock/ass", "shit6");
        add_node(disk_buffer, "/cock/ass", "shit7");
        add_node(disk_buffer, "/cock/ass", "shit8");
        add_node(disk_buffer, "/cock/ass", "shit9");
        add_node(disk_buffer, "/cock/ass", "shit10");
        
        // add_node(disk_buffer, "/", "a");
        // add_node(disk_buffer, "/a", "b");
        // add_node(disk_buffer, "/a/b", "c");
        // add_node(disk_buffer, "/a/b/c", "d");
        // add_node(disk_buffer, "/", "e");
        // add_node(disk_buffer, "/e", "f");
        
        print_fs(disk_buffer);
        
        // printf("node %p\n", find_node(disk_buffer, "/a", NULL));
        // printf("node %p\n", find_node(disk_buffer, "/b", NULL));
        // printf("node %p\n", find_node(disk_buffer, "/c", NULL));
        // printf("node %p\n", find_node(disk_buffer, "/d", NULL));
        // printf("node %p\n", find_node(disk_buffer, "/a/b", NULL));
        // printf("node %p\n", find_node(disk_buffer, "/a/c", NULL));
        // printf("node %p\n", find_node(disk_buffer, "/a/d", NULL));
        // printf("node %p\n", find_node(disk_buffer, "/a/b/c", NULL));
        // printf("node %p\n", find_node(disk_buffer, "/a/b/d", NULL));
        // printf("node %p\n", find_node(disk_buffer, "/a/a", NULL));
        // printf("node %p\n", find_node(disk_buffer, "/a/b/a", NULL));
        // printf("node %p\n", find_node(disk_buffer, "/a/b/c/d", NULL));
    
        FILE *disk_file = fopen("disk.pup", "wb");
        fwrite(disk_buffer, block_size, block_count, disk_file);  
        fclose(disk_file);
        free(disk_buffer);
        
        printf("Ok!\n");
        
        return 0;
    }
    
    printf("no options given\n");
    return -1;
}
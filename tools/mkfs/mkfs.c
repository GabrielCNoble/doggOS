#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include "mkfs.h"

// #include "fs/defs.h"
#include "fs/pup.h"
#include "fs/fs.h"
#include "dsk/dsk.h"
#include "dsk/ram.h"

// enum ARGS
// {
//     ARG_BLOCK_COUNT,
//     ARG_BLOCK_SIZE,
//     ARG_FILE,
//     ARG_MOUNT,
//     ARG_INTERACTIVE,
//     ARG_LAST,
// };

enum CMDS
{
    CMD_INTERACTIVE,
    CMD_MOUNT,
    CMD_CREATE,
    CMD_CD,
    CMD_DIR,
    CMD_SAVE,
    // CMD_MKDIR,
    CMD_QUIT,
    CMD_TREE,
    // CMD_HELP,
    CMD_LAST
};

// enum CMD_CREATE_ARGS
// {
//     ARG_BLOCK_SIZE,
//     ARG_BLOCK_COUNT
// };

// enum CMD_MOUNT_ARGS
// {
//     ARG_IMAGE_FILE,
// };

// struct arg_t
// {
//     char *          name;
// };

struct cmd_t
{
    char *          name;
    uint32_t        arg_count;
    // struct arg_t *  args;
};

struct cmd_t cmds[] = {
    [CMD_INTERACTIVE] = {
        .name = "interactive",
        .arg_count = 0,
        // .args = NULL
    },
    [CMD_MOUNT] = {
        .name = "mount",
        .arg_count = 1,
        // .args = (struct arg_t []) {
        //     [ARG_IMAGE_FILE] = { .name = "image" }
        // }
    },
    [CMD_CREATE] = {
        .name = "create",
        .arg_count = 1,
        // .args = (struct arg_t []) {
        //     [ARG_BLOCK_SIZE] = { .name = "block_size" },
        //     [ARG_BLOCK_COUNT] = { .name = "block_count" },
        // }
    },
    [CMD_CD] = {
        .name = "cd",
        .arg_count = 1,
        // .args = (struct arg_t []) {
        //     [0] = { .name = "path" }
        // }
    },
    [CMD_DIR] = {
        .name = "dir",
    },
    [CMD_QUIT] = {
        .name = "quit",
    },
    [CMD_SAVE] = {
        .name = "save",
        .arg_count = 1,
    },
    [CMD_TREE] = {
        .name = "tree"
    }
};

uint32_t interactive = 0;
struct k_dev_disk_t ram_disk = {
    .type = K_DSK_TYPE_RAM,
    .block_size = 1,
    .read = k_dsk_RamRead,
    .write = k_dsk_RamWrite,
    .clear = k_dsk_RamClear,
};
struct k_fs_vol_t *volume;
struct k_fs_pup_link_t cur_dir = K_FS_PUP_NULL_LINK;
char cur_path[512];

// struct k_fs_pup_range_t find_free_block_range(uint8_t *disk_buffer, uint32_t count, uint32_t usage)
// {
//     // struct k_fs_pup_root_t *root = (struct k_fs_pup_root_t *)disk_buffer;
//     // struct k_fs_pup_range_t range = {};
//     // uint32_t first_block = 0;
//     // uint32_t block_count = 0;
//     // uint8_t status_mask = 0;
//     // uint8_t alloc_mask = 0;
    
//     // switch(usage)
//     // {
//     //     case K_FS_PUP_BLOCK_STATUS_DATA:
//     //         status_mask = K_FS_PUP_BLOCK_STATUS_FREE;
//     //     break;
        
//     //     case K_FS_PUP_BLOCK_STATUS_NODE:
//     //         status_mask = K_FS_PUP_BLOCK_STATUS_NODE;
//     //     break;
//     // }
    
//     // for(uint32_t block_index = 0; block_index < root->bitmask_block_count; block_index++)
//     // {
//     //     uint8_t *bitmask_block = disk_buffer + (root->bitmask_block_start + block_index) * root->block_size;
        
//     //     for(uint32_t byte_index = 0; byte_index < root->block_size; byte_index++)
//     //     {    
//     //         for(uint32_t pair_shift = 0; pair_shift < 8; pair_shift += K_FS_PUP_FREE_BITMASK_BLOCK_BITS)
//     //         {
//     //             uint8_t bitmask_byte = (bitmask_block[byte_index] >> pair_shift) & 0x3;
                
//     //             if(bitmask_byte == K_FS_PUP_BLOCK_STATUS_FREE || bitmask_byte == status_mask)
//     //             {
//     //                 if(!first_block)
//     //                 {
//     //                     first_block = root->available_block_start;
//     //                     first_block += root->block_size * K_FS_PUP_FREE_BITMASK_BLOCKS_PER_BYTE * block_index;
//     //                     first_block += byte_index * K_FS_PUP_FREE_BITMASK_BLOCKS_PER_BYTE;
//     //                     // range.first_block += (root->block_size * block_index + byte_index) * K_FS_PUP_FREE_BITMASK_BLOCKS_PER_BYTE;
//     //                     first_block += pair_shift >> 1;
//     //                     block_count = 0;
//     //                 }
                    
//     //                 block_count++;
                    
//     //                 if(block_count == count)
//     //                 {
//     //                     // printf("return range %lld, %lld\n", range.first_block, range.block_count);
//     //                     range.first_count = K_FS_PUP_RANGE_FIRST_COUNT(first_block, block_count);
//     //                     return range;
//     //                 }
//     //             }
                
//     //             first_block = 0;
//     //         }
//     //     }
//     // }
    
//     // return (struct k_fs_pup_range_t){};
// }

// void set_range_status(uint8_t *disk_buffer, struct k_fs_pup_range_t *range, uint32_t status)
// {
//     // struct k_fs_pup_root_t *root = (struct k_fs_pup_root_t *)disk_buffer;
//     // uint32_t range_first_block = K_FS_PUP_RANGE_FIRST_BLOCK(range->first_count);
//     // uint32_t range_block_count = K_FS_PUP_RANGE_BLOCK_COUNT(range->first_count);
//     // if(range_first_block)
//     // {
//     //     uint32_t blocks_per_bitmask_block = root->block_size * K_FS_PUP_FREE_BITMASK_BLOCKS_PER_BYTE;
//     //     uint32_t first_block = range_first_block - root->available_block_start;
//     //     uint32_t last_block = range_first_block + range_block_count;
        
//     //     for(uint32_t block_index = first_block; block_index < last_block; block_index++)
//     //     {
//     //         uint32_t bitmask_block_index = block_index / blocks_per_bitmask_block;
//     //         uint32_t bitmask_byte_index = (block_index % blocks_per_bitmask_block) >> K_FS_PUP_FREE_BITMASK_BLOCK_BITS;
//     //         uint8_t *block_bitmask = disk_buffer + (root->bitmask_block_start + bitmask_block_index) * root->block_size;
//     //         uint32_t status_shift = K_FS_PUP_FREE_BITMASK_BLOCK_BITS * (block_index & 0x3);
//     //         block_bitmask[bitmask_byte_index] &= ~(K_FS_PUP_BLOCK_STATUS_FULL << status_shift);
//     //         block_bitmask[bitmask_byte_index] |= status << status_shift;
//     //     }
//     // }
// }

// struct k_fs_pup_range_t alloc_blocks(uint8_t *disk_buffer, uint32_t count)
// {
//     // struct k_fs_pup_root_t *root = (struct k_fs_pup_root_t *)disk_buffer;
//     // // printf("alloc_blocks\n");
//     // struct k_fs_pup_range_t range = find_free_block_range(disk_buffer, count, K_FS_PUP_BLOCK_STATUS_DATA);
//     // uint32_t first_block = K_FS_PUP_RANGE_FIRST_BLOCK(range.first_count);
//     // if(first_block)
//     // {
//     //     set_range_status(disk_buffer, &range, K_FS_PUP_BLOCK_STATUS_DATA);
//     // }
    
//     // return range;
// }

// struct k_fs_pup_link_t alloc_node(uint8_t *disk_buffer, uint32_t type)
// {
//     // struct k_fs_pup_root_t *root = (struct k_fs_pup_root_t *)disk_buffer;
//     // struct k_fs_pup_link_t link = {};
//     // // printf("alloc_node\n");
//     // struct k_fs_pup_range_t range = find_free_block_range(disk_buffer, 1, K_FS_PUP_BLOCK_STATUS_NODE);
//     // uint32_t first_block = K_FS_PUP_RANGE_FIRST_BLOCK(range.first_count);
//     // if(first_block)
//     // {
//     //     set_range_status(disk_buffer, &range, K_FS_PUP_BLOCK_STATUS_NODE);
        
//     //     struct k_fs_pup_node_t *node_block = (struct k_fs_pup_node_t *)(disk_buffer + first_block * root->block_size);
//     //     uint32_t node_count = root->block_size / sizeof(struct k_fs_pup_node_t);

//     //     for(uint32_t node_index = 0; node_index < node_count; node_index++)
//     //     {    
//     //         if(node_block[node_index].type == K_FS_PUP_NODE_TYPE_NONE)
//     //         {
//     //             node_block[node_index].type = type;
//     //             node_block[node_index].flags = 0;
                
//     //             // printf("block index: %d\n", range.first_block);
//     //             // printf("node index: %d\n", node_index);
//     //             // struct k_fs_pup_node_t *node = node_block + node_index;
//     //             // node->type = type;
//     //             // node->flags = 0;
//     //             // printf("node %p, type %d, %d\n", node, node->type, type);
//     //             link.link = node_index;
//     //             link.link |= first_block << root->node_index_shift;
//     //             // printf("alloc node %llx\n", link.link);
//     //             // printf("%d\n", root->node_index_shift);
//     //             break;
//     //         }
//     //     }
//     // }
    
//     // return link;
// }

// struct k_fs_pup_node_t *get_node(uint8_t *disk_buffer, struct k_fs_pup_link_t node_address)
// {
//     // struct k_fs_pup_root_t *root = (struct k_fs_pup_root_t *)disk_buffer;
//     // struct k_fs_pup_node_t *node = NULL;
    
//     // if(node_address.link)
//     // {
//     //     uint64_t node_block_index = node_address.link >> root->node_index_shift;
//     //     uint64_t node_index_mask = (1 << root->node_index_shift) - 1;
//     //     uint64_t node_index = node_address.link & node_index_mask;
//     //     // printf("block index: %d\n", node_block_index);
//     //     // printf("node index: %d\n", node_index);
//     //     struct k_fs_pup_node_t *node_block = (struct k_fs_pup_node_t *)(disk_buffer + node_block_index * root->block_size);
//     //     node = node_block + node_index;
//     // }
//     // return node;
// }
 
// void add_node(uint8_t *disk_buffer, char *parent_path, char *node_name)
// {
//     // struct k_fs_pup_root_t *root = (struct k_fs_pup_root_t *)disk_buffer;
//     // struct k_fs_pup_link_t parent_link = K_FS_PUP_NULL_LINK;
//     // struct k_fs_pup_node_t *parent_node = find_node(disk_buffer, parent_path, NULL, &parent_link);
//     // // printf("parent node: %p, %d\n", parent_node, parent_node->type);
//     // if(parent_node && parent_node->type == K_FS_PUP_NODE_TYPE_DIR)
//     // {
//     //     // printf("found parent %s (%p)\n", parent_path, parent_node);
//     //     struct k_fs_pup_node_t *child_node = find_node(disk_buffer, node_name, parent_node, NULL);
        
//     //     if(!child_node) 
//     //     {
//     //         struct k_fs_pup_link_t node_link = alloc_node(disk_buffer, K_FS_PUP_NODE_TYPE_DIR);
            
//     //         for(uint32_t range_index = 0; range_index < K_FS_PUP_MAX_RNODE_RANGES; range_index++)
//     //         {
//     //             uint32_t first_block = K_FS_PUP_RANGE_FIRST_BLOCK(parent_node->ranges[range_index].first_count);
//     //             struct k_fs_pup_dirlist_t *entry_list = (struct k_fs_pup_dirlist_t *)(disk_buffer + first_block * root->block_size);
//     //             struct k_fs_pup_range_t *range = parent_node->ranges + range_index;
//     //             uint32_t range_block_count = K_FS_PUP_RANGE_BLOCK_COUNT(range->first_count);
//     //             uint32_t max_entries = (range_block_count * root->block_size - sizeof(uint32_t)) / sizeof(struct k_fs_pup_dirent_t);
//     //             // uint32_t max_entries = (parent_node->ranges[range_index].block_count * root->block_size - sizeof(uint32_t)) / sizeof(struct k_fs_pup_dirent_t);
                
//     //             if(entry_list->used_count < max_entries)
//     //             {
//     //                 parent_node->child_count++;
                    
//     //                 struct k_fs_pup_dirent_t *entry = entry_list->entries + entry_list->used_count;
                    
//     //                 strcpy(entry->name, node_name);
//     //                 entry->node = node_link;
//     //                 entry_list->used_count++;
                    
//     //                 struct k_fs_pup_node_t *node = get_node(disk_buffer, node_link);
//     //                 node->ranges[0] = alloc_blocks(disk_buffer, 1);
//     //                 node->child_count = 0;
//     //                 node->parent = parent_link;
//     //                 break;
//     //             }
//     //         }
//     //     }
//     // }
// }

// void free_node(uint8_t *disk_buffer, struct k_fs_pup_link_t link)
// {
//     // struct k_fs_pup_root_t *root = (struct k_fs_pup_root_t *)disk_buffer;
//     // uint64_t node_block_index = node_address >> root->node_index_shift;
//     // uint64_t node_index_mask = ~((1 << root->node_index_shift) - 1);
//     // uint64_t node_index = node_address & node_index_mask;
//     // struct k_fs_pup_node_t *node_block = (struct k_fs_pup_node_t *)(disk_buffer + (root->available_block_start + node_block_index) * root->block_size);
//     // struct k_fs_pup_node_t *node = node_block + node_index;
//     // node->type = K_FS_PUP_NODE_TYPE_NONE;
    
//     // uint8_t *bitmask_block = disk_buffer + (root->bitmask_block_start + node_block_index)
// }

// struct k_fs_pup_node_t *find_node(uint8_t *disk_buffer, char *path, struct k_fs_pup_node_t *start_node, struct k_fs_pup_link_t *link)
// {
//     // struct k_fs_pup_root_t *root = (struct k_fs_pup_root_t *)disk_buffer;
//     // struct k_fs_pup_node_t *node = NULL;
//     // struct k_fs_pup_link_t node_link = K_FS_PUP_NULL_LINK;
//     // uint32_t path_cursor = 0;
    
//     // char path_fragment[512];
//     // uint32_t path_fragment_cursor = 0;
    
//     // // while(path[path_cursor] == ' ')
//     // // {
//     // //     path_cursor++;
//     // // }
    
//     // if(!start_node && path[path_cursor] == '/')
//     // {
//     //     path_cursor++;
//     //     node = get_node(disk_buffer, root->root_node);
//     //     node_link = root->root_node;
//     // }
//     // else
//     // {
//     //     node = start_node;
//     //     // node_link = start_node;
//     // }
        
//     // while(path[path_cursor] && node)
//     // {
//     //     path_fragment_cursor = 0;
//     //     while(path[path_cursor] != '/' && path[path_cursor])
//     //     {
//     //         path_fragment[path_fragment_cursor] = path[path_cursor];
//     //         path_fragment_cursor++;
//     //         path_cursor++;
//     //     }
        
//     //     if(path[path_cursor] == '/')
//     //     {
//     //         path_cursor++;
//     //     }
        
//     //     path_fragment[path_fragment_cursor] = '\0';
//     //     struct k_fs_pup_node_t *next_node = NULL;
//     //     struct k_fs_pup_link_t next_node_link = K_FS_PUP_NULL_LINK;
        
//     //     // printf("node type: %d\n", node->type);
        
//     //     if(node->type == K_FS_PUP_NODE_TYPE_DIR)
//     //     {                
//     //         // printf("%s\n", path_fragment);
            
//     //         for(uint32_t range_index = 0; range_index < K_FS_PUP_MAX_RNODE_RANGES && !next_node; range_index++)
//     //         {
//     //             struct k_fs_pup_range_t *range = node->ranges + range_index;
//     //             uint32_t first_block = K_FS_PUP_RANGE_FIRST_BLOCK(range->first_count);
//     //             struct k_fs_pup_dirlist_t *entry_list = (struct k_fs_pup_dirlist_t *)(disk_buffer + first_block * root->block_size);
//     //             // uint32_t entry_count = range->block_count * (root->block_size / sizeof(struct k_fs_pup_direntry_t));
//     //             // printf("blah\n");
//     //             for(uint32_t entry_index = 0; entry_index < entry_list->used_count; entry_index++)
//     //             {
//     //                 struct k_fs_pup_dirent_t *entry = entry_list->entries + entry_index;
//     //                 // printf("%s\n", entry->name);
                    
//     //                 if(!entry->node.link)
//     //                 {
//     //                     break;
//     //                 }
                    
//     //                 if(!strcmp(path_fragment, entry->name))
//     //                 {
//     //                     // printf("%s\n", entry->name);
//     //                     next_node = get_node(disk_buffer, entry->node);
//     //                     next_node_link = entry->node;
//     //                     // printf("%p - %s\n", next_node, entry->name);
//     //                     break;
//     //                 }
//     //             }
//     //         }
//     //     }
        
//     //     node = next_node;
//     //     node_link = next_node_link;
//     //     // printf("%p\n", node);
//     // }
//     // // }
//     // if(link)
//     // {
//     //     *link = node_link;
//     // }
//     // return node;
// }


void error(char *format, ...)
{
    va_list arg_list;
    va_start(arg_list, format);
    printf("ERROR: ");
    vprintf(format, arg_list);
    printf("\n");

    if(!interactive)
    {
        exit(-1);
    }
}

// uint32_t cmd_arg(struct cmd_t *cmd, char *arg_name)
// {
//     for(uint32_t index = 0; index < cmd->arg_count; index++)
//     {
//         if(!strcmp(cmd->args[index].name, arg_name))
//         {
//             return index;
//         }
//     }

//     return 0xffffffff;
// }

void parse_arg(char *cmd_str, uint32_t *cursor, char **arg_name, char **arg_value)
{
    uint32_t local_cursor = *cursor;

    *arg_name = NULL;
    *arg_value = NULL;

    /* skip all spaces after the command or previous arg */
    while(cmd_str[local_cursor] == ' ' && cmd_str[local_cursor] != '\0')
    {
        local_cursor++;
    }

    if(cmd_str[local_cursor] != '\0')
    {
        /* arg name starts here */
        *arg_name = cmd_str + local_cursor;

        while(cmd_str[local_cursor] != '\0' && cmd_str[local_cursor] != '=' && cmd_str[local_cursor] != ' ')
        {
            local_cursor++;
        }

        cmd_str[local_cursor] = '\0';
        local_cursor++;

        /* we may have spaces befoe the '=', so skip those too*/
        while(cmd_str[local_cursor] == ' ' && cmd_str[local_cursor] != '\0')
        {
            local_cursor++;
        }

        /* we had spaces between the arg name and the '=', so skip the '=' */ 
        if(cmd_str[local_cursor] == '=')
        {
            local_cursor++;

            /* we may still have spaces after the '=', so skip those too */
            while(cmd_str[local_cursor] == ' ' && cmd_str[local_cursor] != '\0')
            {
                local_cursor++;
            }
        }

        if(cmd_str[local_cursor] != '\0')
        {   
            /* arg value starts here */
            *arg_value = cmd_str + local_cursor;

            while(cmd_str[local_cursor] != '\0' && cmd_str[local_cursor] != '=' && cmd_str[local_cursor] != ' ')
            {
                local_cursor++;
            }

            cmd_str[local_cursor] = '\0';
            local_cursor++;
        }
    }

    // printf("%s\n", cmd_str + local_cursor);

    *cursor = local_cursor;
}

int parse_cmd(char *cmd_str, uint32_t cmd_index)
{
    uint32_t cursor = 0;
    uint32_t cur_cmd = CMD_LAST;
    while(cmd_str[cursor] == ' ') cursor++;

    if(!cmd_str[cursor])
    {
        return CMD_LAST;
    }

    if(cmd_str[0] != '-')
    {
        error("Unknown command [%s] at position %d", cmd_str, cmd_index);
        return cur_cmd;
    }

    cmd_str++;

    for(uint32_t cmd_index = 0; cmd_index < CMD_LAST; cmd_index++)
    {
        if(strstr(cmd_str, cmds[cmd_index].name) == cmd_str)
        {
            cur_cmd = cmd_index;
            break;
        }
    }

    if(cur_cmd == CMD_LAST)
    {
        error("Unknown command [%s] at position %d", cmd_str, cmd_index);
        return cur_cmd;
    }
    /* skip cmd name */
    while(cmd_str[cursor] != ' ' && cmd_str[cursor] != '\0')
    {
        cursor++;
    }

    struct cmd_t *cmd = cmds + cur_cmd;
    char *arg_name;
    char *arg_value;
    uint32_t arg_count = 0;

    switch(cur_cmd)
    {
        case CMD_CREATE:
        {
            uint32_t block_size = 512;
            uint32_t block_count = 0;

            for(uint32_t arg_index = 0; arg_index < cmd->arg_count; arg_index++)
            {
                parse_arg(cmd_str, &cursor, &arg_name, &arg_value);

                if(!arg_name)
                {
                    break;
                }

                if(arg_value)
                {
                    if(!strcmp(arg_name, "block_count"))
                    {
                        block_count = atoi(arg_value);
                    }
                }
            }

            create_image(block_size, block_count);
        }
        break;

        case CMD_MOUNT:
        {
            char *image_name = NULL;

            parse_arg(cmd_str, &cursor, &arg_name, &arg_value);

            if(arg_name && !strcmp(arg_name, "image"))
            {
                image_name = arg_value;
            }

            load_image(image_name);
        }
        break;

        case CMD_SAVE:
        {
            char *image_name = NULL;

            parse_arg(cmd_str, &cursor, &arg_name, &arg_value);

            if(arg_name && !strcmp(arg_name, "image"))
            {
                image_name = arg_value;
            }

            save_image(image_name);
        }
        break;

        case CMD_DIR:
            dir();
        break;

        case CMD_CD:
        {
            char *path = NULL;

            parse_arg(cmd_str, &cursor, &arg_name, &arg_value);

            if(!arg_name || !arg_value || strcmp(arg_name, "path"))
            {
                break;
            }

            path = arg_value;
            cd(path);
        }
        break;

        case CMD_TREE:
            tree();
        break;
    }

    return cur_cmd;
}

void create_image(uint32_t block_size, uint32_t block_count)
{
    if(!block_count)
    {
        error("No block count provided!");
        return;
    }
    
    // if(block_size < 512)
    // {
    //     error("Block size should be at least 512 bytes!");
    //     return;
    // }
    
    // for(uint32_t test_mask = 0x80000000; test_mask; test_mask >>= 1)
    // {
    //     if((block_size & test_mask) && (block_size & (test_mask - 1)))
    //     {
    //         error("Block size is not a power of two!");
    //         return;
    //     }
    // }
    block_size = K_FS_PUP_LOGICAL_BLOCK_SIZE;
    void *image_buffer = calloc(block_count, block_size);
    init_disk(image_buffer, block_size * block_count);
    // mount_image(image_buffer, block_count * block_size);

    struct k_fs_pup_format_args_t format_args = {
        .block_size = block_size,
        .block_count = block_count
    };

    struct k_fs_part_t partition = {
        .disk = &ram_disk,
        .first_block = 0,
        .block_count = block_size * block_count,
    };

    // k_sys_TerminalPrintf("%d\n", offsetof(struct k_fs_pup_node_t, parent));

    cur_dir = K_FS_PUP_NULL_LINK;
    volume = k_fs_FormatPartition(&partition, K_FS_FILE_SYSTEM_PUP, &format_args);
    cd("/");
}

void init_disk(void *disk_buffer, uint32_t disk_size)
{
    clear_disk();
    ram_disk.start_address = (uintptr_t)disk_buffer;
    ram_disk.block_count = disk_size;
    printf("initialize disk: %d blocks, %d bytes per block\n", ram_disk.block_count, ram_disk.block_size);

    // ram_disk.type = K_DSK_TYPE_RAM;
    // ram_disk.start_address = (uintptr_t)disk_buffer;
    // ram_disk.block_size = 1;
    // ram_disk.block_count = disk_size;
    // ram_disk.read = k_dsk_Ram_Read;
    // ram_disk.write = k_dsk_Ram_Write;
    // ram_disk.clear = k_dsk_Ram_Clear;
}

void clear_disk()
{
    if(ram_disk.start_address)
    {
        free((void *)ram_disk.start_address);
    }

    ram_disk.start_address = 0;
    ram_disk.block_count = 0;
}

void mount_image()
{
    struct k_fs_part_t partition = {
        .disk = &ram_disk,
        .first_block = 0,
        .block_count = ram_disk.block_size * ram_disk.block_count
    };

    volume = k_fs_MountVolume(&partition);
    printf("volume mounted\n");
    cd("/");
}

void load_image(char *image_file)
{
    if(!image_file)
    {
        error("No file system image file provided!");
        return;
    }

    FILE *file = fopen(image_file, "rb");

    if(!file)
    {
        error("Couldn't find file system image file [%s]", image_file);
        return;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    void *image_buffer = calloc(1, file_size);
    fread(image_buffer, 1, file_size, file);
    fclose(file);

    init_disk(image_buffer, file_size);
    mount_image();
    printf("file system image [%s] mounted\n", image_file);
}

void save_image(char *image_file)
{
    if(!image_file)
    {
        error("No file system image name provided!");
        return;
    }

    FILE *file = fopen(image_file, "wb");
    if(!file)
    {
        error("Couldn't open file system image file [%s]", image_file);
        return;
    }

    fwrite((void *)ram_disk.start_address, ram_disk.block_size, ram_disk.block_count, file);
    fclose(file);
}

void dir()
{
    if(volume)
    {
        // struct k_fs_pup_link_t node = k_fs_PupFindNode(volume, "/", K_FS_PUP_NULL_LINK, NULL);
        // struct k_fs_pup_dirlist_t *dir_list = k_fs_PupGetNodeDirList(volume, "/", cur_dir);
        printf("contents of directory [%s]\n", cur_path);

        // if(dir_list)
        // {
        //     for(uint32_t entry_index = 0; entry_index < dir_list->used_count; entry_index++)
        //     {
        //         struct k_fs_pup_dirent_t *entry = dir_list->entries + entry_index;
        //         printf("    %s\n", entry->name);
        //     }
        // }
    }
    else
    {
        error("No file system mounted!");
    }
}

void cd(char *path)
{
    if(volume)
    {
        cur_dir = k_fs_PupFindNode(volume, path, cur_dir, NULL);
        k_fs_PupGetPathToNode(volume, cur_dir, cur_path, sizeof(cur_path));
    }
    else
    {
        error("No file system mounted!");
    }
}

void tree(uint8_t *disk_buffer)
{
    if(volume)
    {
        struct k_fs_pup_vol_t *pup_volume = (struct k_fs_pup_vol_t *)volume->data;
        // struct k_fs_pup_centry_t *entry = k_fs_PupAllocCacheEntry(pup_volume);
        struct k_fs_pup_centry_t *entry = k_fs_PupLoadEntry(volume, K_FS_PUP_NODE_BLOCK(pup_volume->root.root_node), 1, NULL);
        struct k_fs_pup_node_t *root_node = k_fs_PupGetNode(volume, pup_volume->root.root_node, entry);
        struct k_fs_pup_dirent_t dir_entry = {
            .link = pup_volume->root.root_node,
            .left = 0xff,
            .right = 0xff,
            .name = "root"
        };

        print_node_contents(NULL, &dir_entry, 0);

        k_fs_PupReleaseEntry(entry);
        // print_node(root_node, 0);
    }
    else
    {
        error("No file system mounted!");
    }
}

void print_node_contents(struct k_fs_pup_content_t *content, struct k_fs_pup_dirent_t *entry, uint32_t depth)
{
    // if(entry->left & entry->right == 0xff)
    // {
    //     if(entry->link.link != 0)
    //     {
    //         // content = (struct k_fs_pup_content_t *)(disk_buffer + entry->link.link * K_FS_PUP_LOGICAL_BLOCK_SIZE);
    //         // struct k_fs_pup_centry_t *content_entry = k_fs_PupGetBlock(volume, NULL, entry->link);
    //         content = k_fs_PupGetBlock(volume, NULL, entry->link);
    //         entry = &content->dir_entries[0];
    //         print_node_contents(content, entry, depth);    
    //     }
    //     return;
    // }

    if(entry->left != 0xff)
    {
        print_node_contents(content, &content->dir_entries[entry->left], depth);
    }

    for(uint32_t depth_index = 0; depth_index < depth; depth_index++)
    {
        putchar(' ');
    }
    
    putchar('|');
    putchar('-');
    
    printf("%s\n", entry->name);
    struct k_fs_pup_centry_t *node_entry = k_fs_PupLoadEntry(volume, K_FS_PUP_NODE_BLOCK(entry->link), 1, NULL);
    struct k_fs_pup_node_t *node = k_fs_PupGetNode(volume, entry->link, node_entry);
    print_node(node, depth + 1);
    k_fs_PupReleaseEntry(node_entry);

    if(entry->right != 0xff)
    {
        print_node_contents(content, &content->dir_entries[entry->right], depth);
    }
}

void print_node(struct k_fs_pup_node_t *node, uint32_t depth)
{
    if(node != NULL && node->type == K_FS_PUP_NODE_TYPE_DIR)
    {
        if(node->contents.link != 0)
        {
            struct k_fs_pup_centry_t *entry = k_fs_PupLoadEntry(volume, node->contents.link, 1, NULL);
            struct k_fs_pup_content_t *content = k_fs_PupGetBlock(volume, entry, node->contents);
            print_node_contents(content, &content->dir_entries[0], depth);
            k_fs_PupReleaseEntry(entry);
        }
    }
}

int main(int argc, char *argv[])
{
    if(argc > 1)
    {
        uint32_t cur_cmd = CMD_LAST;
        uint32_t cmd_index = 0;
        char *cmd_str_buffer = calloc(1, 0xffff);
        uint32_t cmd_str_buffer_cursor = 0;

        for(uint32_t arg_index = 1; arg_index < argc; arg_index++)
        {
            char *arg_str = argv[arg_index];
            uint32_t str_cursor = 0;

            keep_copying_same_argv:

            while(arg_str[str_cursor] != '\0' && arg_str[str_cursor] != ',')
            {
                cmd_str_buffer[cmd_str_buffer_cursor] = arg_str[str_cursor];
                cmd_str_buffer_cursor++;
                str_cursor++;
            }

            if(arg_str[str_cursor] != ',' && arg_index < argc - 1)
            {
                /* we're still not done copying the args for the current command, and 
                we're also not in the last cmd line argument, so move to the next cmd
                line arg */
                cmd_str_buffer[cmd_str_buffer_cursor] = ' ';
                cmd_str_buffer_cursor++;
                continue;
            }


            cmd_str_buffer[cmd_str_buffer_cursor] = '\0';
            if(parse_cmd(cmd_str_buffer, cmd_index) == CMD_INTERACTIVE)
            {
                printf("Tool is now in interactive mode.\n");
                interactive = 1;
                /* the moment we enter interactive mode, we ignore the rest of cmd line args */
                break;
            }

            cmd_str_buffer_cursor = 0;
            cmd_index++;

            if(arg_str[str_cursor] == ',' && arg_str[str_cursor + 1] != '\0')
            {
                str_cursor++;
                // while(arg_str[str_cursor] == ' ') str_cursor++;
                /* we may have part of the next command in this cmd line argument, so
                keep copying from it */
                goto keep_copying_same_argv;
            }
        }

        while(interactive)
        {
            if(volume)
            {
                printf("%s ", cur_path);
            }
            printf("> ");
            fgets(cmd_str_buffer, 0xffff, stdin);
            uint32_t cmd_len = strlen(cmd_str_buffer);
            cmd_str_buffer[cmd_len - 1] = '\0';
            if(parse_cmd(cmd_str_buffer, 0) == CMD_QUIT)
            {
                interactive = 0;
            }
        }

        printf("Bye\n");

        free(cmd_str_buffer);
    }
    else
    {
        printf("No options given. Type -help for... help.\n");
    }

    return 0;
}


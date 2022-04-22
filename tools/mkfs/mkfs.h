#ifndef MKFS_H
#define MKFS_H

#include "../../kernel/fs/pup.h"

struct k_fs_pup_range_t find_free_block_range(uint8_t *disk_buffer, uint32_t count, uint32_t usage);

void set_range_status(uint8_t *disk_buffer, struct k_fs_pup_range_t *range, uint32_t status);

struct k_fs_pup_range_t alloc_blocks(uint8_t *disk_buffer, uint32_t count);

struct k_fs_pup_link_t alloc_node(uint8_t *disk_buffer, uint32_t type);

struct k_fs_pup_node_t *get_node(uint8_t *disk_buffer, struct k_fs_pup_link_t node_address);

void add_node(uint8_t *disk_buffer, char *parent_path, char *node_name);

void free_node(uint8_t *disk_buffer, struct k_fs_pup_link_t link);

struct k_fs_pup_node_t *find_node(uint8_t *disk_buffer, char *path, struct k_fs_pup_node_t *start_node, struct k_fs_pup_link_t *link);

void print_fs(uint8_t *disk_buffer);

#endif
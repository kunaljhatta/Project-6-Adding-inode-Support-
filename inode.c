#include "inode.h"
#include "block.h"
#include "free.h"
#include "pack.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static struct inode incore[MAX_SYS_OPEN_FILES] = {0};

struct inode *find_incore_free(void){
    for(int i=0; i<MAX_SYS_OPEN_FILES; i++){
        if(incore[i].ref_count == 0){
            return incore + i;
        }
    }
    return NULL;
}

struct inode *find_incore(unsigned int inode_num){
    for(int i=0; i<MAX_SYS_OPEN_FILES; i++){
        if(incore[i].ref_count != 0 && incore[i].inode_num == inode_num)
        {
            return incore + i;}
    }
    return NULL;
}

void read_inode(struct inode *in, int inode_num){
    int block_num = inode_num / INODE_SIZE + INODE_FIRST_BLOCK;
    int block_offset = inode_num % INODE_SIZE;
    int block_offset_bytes = block_offset * INODE_SIZE;

    unsigned char inode_block[BLOCK_SIZE] = {0};

    bread(block_num, inode_block);
    in->size = read_u32(inode_block + block_offset_bytes);
    in->owner_id = read_u16(inode_block + block_offset_bytes + ID_OFFSET);
    in->permissions = read_u8(inode_block + block_offset_bytes + PERMISSIONS_OFFSET);
    in->flags = read_u8(inode_block + block_offset_bytes + FLAGS_OFFSET);
    in->link_count = read_u8(inode_block + block_offset_bytes + LINK_COUNT_OFFSET);
    for(int i = 0; i < INODE_PTR_COUNT; i++) {
        in->block_ptr[i] = read_u16(inode_block + block_offset_bytes + 9 + (i*2));
    }
}

void write_inode(struct inode *in){
	int inode_num = in->inode_num;
	int block_num = inode_num / INODES_PER_BLOCK + INODE_FIRST_BLOCK;
	int block_offset = inode_num % INODES_PER_BLOCK;
	int block_offset_bytes = block_offset * INODE_SIZE;

    unsigned char inode_block[BLOCK_SIZE] = {0};

    write_u32(inode_block + block_offset_bytes, in->size);
    write_u16(inode_block + block_offset_bytes + ID_OFFSET, in->owner_id);
    write_u8(inode_block + block_offset_bytes + PERMISSIONS_OFFSET, in->permissions);
    write_u8(inode_block + block_offset_bytes + FLAGS_OFFSET, in->flags);
    write_u8(inode_block + block_offset_bytes + LINK_COUNT_OFFSET, in->link_count);
    int block_pointer_address = BLOCK_PTR_OFFSET;
    for (int i = 0; i < INODE_PTR_COUNT; i++) {
    	write_u16(inode_block + block_offset_bytes + block_pointer_address, in->block_ptr[i]);
    	block_pointer_address += 2;
    }
	bwrite(block_num, inode_block);
}

struct inode *iget(int inode_num){
    struct inode *incore_node = find_incore(inode_num);
    if(incore_node != NULL){
        incore_node->ref_count++;
        return incore_node;
    }
    struct inode *incore_free= find_incore_free();
    if(incore_free == NULL){
        return NULL;
    }
    read_inode(incore_free, inode_num);
    incore_free->ref_count = 1;
    incore_free->inode_num = inode_num;
    return incore_free;
}

void iput(struct inode *in){
    if(in->ref_count == 0){
        return;
    }
    in->ref_count--;
    if(in->ref_count == 0){
        write_inode(in);
    }
}

void reset_incore_inodes(void){
    for(int i=0; i<MAX_SYS_OPEN_FILES; i++){
        incore[i].ref_count = 0;
    }
}

struct inode *ialloc(void){
    
    unsigned char inode_block[BLOCK_SIZE] = {0};

    bread(FREE_INODE_BLOCK_NUM, inode_block);
    int free_num = find_free(inode_block);
    if(free_num == -1){
        return NULL;
    }

    set_free(inode_block, free_num, 1);
    bwrite(FREE_INODE_BLOCK_NUM, inode_block);

    struct inode *node = iget(free_num);
    if(node == NULL){
        return NULL;
    }
    node->size = 0;
    node->owner_id = 0;
    node->permissions = 0;
    node->flags = 0;

    write_inode(node);

    return node;
}
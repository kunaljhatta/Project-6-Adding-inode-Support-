#ifndef INODE_H
#define INODE_H

#define FREE_INODE_BLOCK_NUM 1
#define INODE_PTR_COUNT 16
#define MAX_SYS_OPEN_FILES 64

#define INODE_SIZE 64
#define INODE_FIRST_BLOCK 3

#define ID_OFFSET 4
#define PERMISSIONS_OFFSET ID_OFFSET + 2
#define FLAGS_OFFSET PERMISSIONS_OFFSET + 1
#define LINK_COUNT_OFFSET FLAGS_OFFSET + 1
#define BLOCK_PTR_OFFSET LINK_COUNT_OFFSET + 1
#define INODES_PER_BLOCK (BLOCK_SIZE / INODE_SIZE)

struct inode *ialloc(void);
struct inode *find_incore_free(void);
struct inode *find_incore(unsigned int inode_num);

void read_inode(struct inode *in, int inode_num);
void write_inode(struct inode *in);
struct inode *iget(int inode_num);
void iput(struct inode *in);
void reset_incore_inodes(void);

struct inode {
    unsigned int size;
    unsigned short owner_id;
    unsigned char permissions;
    unsigned char flags;
    unsigned char link_count;
    unsigned short block_ptr[INODE_PTR_COUNT];

    unsigned int ref_count;  // in-core only
    unsigned int inode_num;
};

#endif
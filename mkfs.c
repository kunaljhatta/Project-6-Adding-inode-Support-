#include "mkfs.h"
#include "inode.h"
#include "block.h"
#include "free.h"
#include "image.h"
#include <fcntl.h>
#include <unistd.h>

void mkfs(void) {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        unsigned char zero_block[BLOCK_SIZE] = {0};
        write(image_fd, zero_block, sizeof(zero_block));
    }
    for (int i = 0; i < 7; i++) {
        alloc();
    }
}
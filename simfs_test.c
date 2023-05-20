#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "free.h"
#include "image.h"
#include "mkfs.h"
#include "ctest.h"
#include "inode.h"
#include "block.h"

void test_image_open(){   
    char *file = "Hello.txt!";
    CTEST_ASSERT((image_open(file, 0) != -1), "File is not there!");
    remove(file);
}

void test_image_close(){
    char *file = "test.txt";
    image_open(file, 1);
    CTEST_ASSERT(image_close() == 0, "");
    remove(file);
}

void test_bwrite_and_bread(){
	unsigned char test[BLOCK_SIZE] = {0}; 
	image_open("anyfile.txt", 0); 
	bwrite(9, test);
	unsigned char read[BLOCK_SIZE];
	bread(9, read);
	CTEST_ASSERT(memcmp(test, read, 3) == 0, "Testing bwrite and bread");
	image_close();
}

void test_alloc(){
    image_open("anyfile.txt", 0);

    CTEST_ASSERT(alloc() == 0, "empty block map");
    CTEST_ASSERT(alloc() == 1, "non-empty block map");

    image_close();
}

void test_set_free_and_find_free(void) {
    unsigned char data[4096] = {0};
    CTEST_ASSERT(find_free(data) == 0, "Testing empty");

    set_free(data, 0, 1);
    CTEST_ASSERT(find_free(data) == 1, "Testing find_free to 1 and set_free to 1");

    set_free(data, 1, 1);
    CTEST_ASSERT(find_free(data) == 2, "Testing find_free to 2 and set_free to 1");

    set_free(data, 0, 0);
    CTEST_ASSERT(find_free(data) == 0, "Testing find_free to 0 and set_free to 0");
}

void test_ialloc(void) {
	image_open("any.txt", 0);

    struct inode *ialloc_node = ialloc();
    CTEST_ASSERT(ialloc_node->inode_num == 0, "Testing empty inode map");
    CTEST_ASSERT(ialloc_node->size == 0, "Testing size attribute");
    CTEST_ASSERT(ialloc_node->owner_id == 0, "Testing owner_id attribute");   
    CTEST_ASSERT(ialloc_node->flags == 0, "Testing flags attribute");   
    CTEST_ASSERT(ialloc_node->permissions == 0, "Testing permissions attribute");
    CTEST_ASSERT(ialloc_node->ref_count == 1, "Testing ref_count attribute");       
    
    struct inode *ialloc_node_two = ialloc();
    CTEST_ASSERT(ialloc_node_two->inode_num == 1, "Testing non-empty inode map");

    image_close();
    remove("any.txt");
}

void test_find_free_incore_and_find_incore()
{
    image_open("test.txt", 1);
    reset_incore_inodes();

    struct inode *free_node = find_incore_free();
    CTEST_ASSERT(free_node != NULL, "Testing that incore has a free inode");
    CTEST_ASSERT(free_node->ref_count == 0, "Testing that the free inode has a reference count of zero");

    iget(0);
    struct inode *free_node_2 = find_incore_free();
    CTEST_ASSERT(free_node_2 != NULL, "Testing that incore has another free inode");
    CTEST_ASSERT(free_node_2 != free_node, "Testing that the next free inode is different from the previously allocated inode");

    iget(1);
    struct inode *node = find_incore(1);
    CTEST_ASSERT(node != NULL, "Testing that incore has the specific inode");
    CTEST_ASSERT(node->inode_num == 1, "Testing that the specific inode has the correct inode number");
    image_close();
    remove("test.txt");
}

void test_read_and_write_inode(void)
{
	image_open("test.txt", 0);
    unsigned int test_inode_num = 314;
	struct inode *new_inode = find_incore_free();
	new_inode->inode_num = test_inode_num;
	new_inode->size = 456;
	new_inode->owner_id = 7;
	new_inode->permissions = 8;
	new_inode->flags = 9;
	new_inode->link_count = 10;
	new_inode->block_ptr[0] = 11;

	write_inode(new_inode);
	struct inode inode_read_buffer = {0};
	read_inode(&inode_read_buffer, test_inode_num);

	CTEST_ASSERT(new_inode->size == 456, "Testing size attribute");
	CTEST_ASSERT(new_inode->owner_id == 7, "Testing owner_id attribute");
	CTEST_ASSERT(new_inode->permissions == 8, "Testing permissions attribute");
	CTEST_ASSERT(new_inode->flags == 9, "Testing flags attribute");
	CTEST_ASSERT(new_inode->link_count == 10, "Testing link_count attribute");
	CTEST_ASSERT(new_inode->block_ptr[0] == 11, "Testing block pointer attribute");

    image_close();
    remove("test.txt");
}

void test_iget()
{
    image_open("test.txt", 1);
    reset_incore_inodes();

    unsigned int inode_num = 10;
    struct inode *new_node = iget(inode_num);
    CTEST_ASSERT(new_node->inode_num == inode_num, "Testing if iget returns an inode with the specified inode_num.");
    CTEST_ASSERT(new_node->ref_count == 1, "Testing iget for a node that is not in the core updates the ref_count to 1");

    struct inode *node_two = iget(inode_num);
    CTEST_ASSERT(node_two->ref_count == 2, "Testing iget for inode increments the ref count correctly");
    CTEST_ASSERT(node_two == new_node, "Testing iget return the same inode pointer");

    image_close();
    remove("test.txt");
}   

void test_iput()
{
	image_open("test.txt", 0);
	struct inode *test_node = iget(1);
	CTEST_ASSERT(test_node->ref_count == 1, "Testing initial ref_count is 1");

	iput(test_node);
	CTEST_ASSERT(test_node->ref_count == 0, "Testing ref_count is decremented to zero");

	struct inode *iget_inode = iget(1);
	CTEST_ASSERT(iget_inode->ref_count == 1, "Testing that iget increments ref_count correctly");
	CTEST_ASSERT(iget_inode->inode_num == test_node->inode_num, "Testing that iget retrieves the correct inode");
	CTEST_ASSERT(iget_inode->size == test_node->size, "Testing that iget retrieves the correct inode");

	image_close();
	remove("test.txt");
}

void test_mkfs(void) {
    image_open("test_file.txt", 0);
    unsigned char test_block[4096] = {0};
    unsigned char test_outblock[4096];
    mkfs();
    CTEST_ASSERT(memcmp(bread(8, test_outblock), test_block, 4) == 0, "setting blocks to 0");

    int next_free_block = alloc();
    CTEST_ASSERT(next_free_block == 7, "testing next free block");

    image_close();
    remove("test_file.txt");
}

int main(void)
{
    CTEST_VERBOSE(1);
    test_image_open();
    test_image_close();
    test_alloc();
    test_bwrite_and_bread();
    test_set_free_and_find_free();
    test_ialloc();
    test_find_free_incore_and_find_incore();
    test_read_and_write_inode();
    test_mkfs();
    test_iget();
    test_iput();
    CTEST_RESULTS();
    CTEST_EXIT();
}

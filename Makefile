.PHONY: test

simfs.a: block.o image.o free.o inode.o mkfs.o pack.o
	ar rcs $@ $^

inode.o: inode.c inode.h
	gcc -Wall -Wextra -c $<

pack.o: pack.c
	gcc -Wall -Wextra -c $<
	
block.o: block.c block.h
	gcc -Wall -Wextra -c $<

image.o: image.c image.h
	gcc -Wall -Wextra -c $<

mkfs.o: mkfs.c mkfs.h
	gcc -Wall -Wextra -c $<

free.o: free.c free.h
	gcc -Wall -Wextra -c $<

simfs_test: simfs_test.c simfs.a
	gcc -Wall -Wextra -DCTEST_ENABLE -o $@ $^

test: simfs_test
	./simfs_test

clean:
	rm -f *.o *.a anyfile.txt *_testfile simfs_test


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <bsd/string.h>
#include <assert.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "storage.h"
#include "slist.h"

// implementation for: man 2 access
// Checks if a file exists.
int
nufs_access(const char *path, int mask)
{
    printf("access(%s, %04o)\n", path, mask);
    if (/*path doesn't exist*/!storage_access(path, mask)) {
        return -ENOENT;
    }
    // TODO:
    else if (/*requested permission isn't available*/0) {
        return -EACCES;
    }
    return 0;
}

// implementation for: man 2 stat
// gets an object's attributes (type, permissions, size, etc)
int
nufs_getattr(const char *path, struct stat *st)
{
    printf("getattr(%s)\n", path);
    int rv = get_stat(path, st);
    if (rv == -1) {
        return -ENOENT;
    }
    else {
        return rv;
    }
}

// implementation for: man 2 readdir
// lists the contents of a directory
int
nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi)
{
    struct stat st;

    printf("readdir(%s)\n", path);
    
    slist* contents = directory_list(path);

    get_stat(path, &st);

    // filler is a callback that adds one item to the result
    // it will return non-zero when the buffer is full
    filler(buf, ".", &st, 0);
    
    while (contents) {
        filler(buf, contents->data, &st, 0);
        contents = contents->next;
    }

    return 0;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
int
nufs_mknod(const char *path, mode_t mode, dev_t rdev)
{
    printf("mknod(%s, %04o)\n", path, mode);
    return storage_mknod(path, mode, rdev);
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int
nufs_mkdir(const char *path, mode_t mode)
{
    printf("mkdir(%s)\n", path);
     
    //TODO:
    if (/*failedrv != */0) {
        return -errno;
    }
    return storage_mkdir(path, mode);
}

// acts as removing a node pointed to by path
int
nufs_unlink(const char *path)
{
    printf("unlink(%s)\n", path);
    return storage_unlink(path);
}

// removes a directory specified by path
int
nufs_rmdir(const char *path)
{
    printf("rmdir(%s)\n", path);
    
    if (0) {
        return -errno;
    }
    return storage_rmdir(path);
}

// implements: man 2 rename
// called to move a file within the same filesystem
int
nufs_rename(const char *from, const char *to)
{
    printf("rename(%sr=> %s)\n", from, to);
    return storage_rename(from, to);
}

int
nufs_chmod(const char *path, mode_t mode)
{
    printf("chmod(%s, %04o)\n", path, mode);
    return -1;
}

int
nufs_truncate(const char *path, off_t size)
{
    printf("truncate(%s, %ld bytes)\n", path, size);
    return storage_truncate(path, size);
}

// this is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
int
nufs_open(const char *path, struct fuse_file_info *fi)
{
    printf("open(%s)\n", path);
    return 0;
}

// Actually read data
int
nufs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    printf("read(%s, %ld bytes, @%ld)\n", path, size, offset);
    return storage_read(path, buf, size, offset);
}

// Actually write data
int
nufs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    printf("write(%s, %ld bytes, @%ld)\n", path, size, offset);
    return storage_write(path, buf, size, offset);//-1;
}

// creates a hard link at to, pointing to the data refrenced by 
// from. TODO: Currently not working. 
int
nufs_link(const char *from, const char *to)
{
    printf("link(%s, %s)\n", from, to);
    return storage_link(from, to);
}

// creates a soft link at to, refrencing the name of the path
// from.
int 
nufs_symlink(const char* from, const char* to)
{
	printf("symlink(%s, %s)\n", from, to);
	return storage_symlink(from, to);
	
}

// reads the data at the end of the link at path. 
int
nufs_readlink(const char* path, char* buff, size_t size)
{
	printf("readlink(%s, %u)\n", path, size);
	return storage_readlink(path, buff, size);//0;
}

int
nufs_fgetattr(const char* path, struct stat* stbuff)
{
	printf("fgetattr(%s)\n", path);
	return -1;
}
void
nufs_init_ops(struct fuse_operations* ops)
{
    memset(ops, 0, sizeof(struct fuse_operations));
    ops->access   = nufs_access;
    ops->getattr  = nufs_getattr;
    ops->readdir  = nufs_readdir;
    ops->mknod    = nufs_mknod;
    ops->mkdir    = nufs_mkdir;
    ops->unlink   = nufs_unlink;
    ops->rmdir    = nufs_rmdir;
    ops->rename   = nufs_rename;
    ops->chmod    = nufs_chmod;
    ops->truncate = nufs_truncate;
    ops->open	  = nufs_open;
    ops->read     = nufs_read;
    ops->write    = nufs_write;
	
    ops->link     = nufs_link;
    ops->symlink  = nufs_symlink;
    ops->readlink = nufs_readlink;
};

struct fuse_operations nufs_ops;

int
main(int argc, char *argv[])
{
    assert(argc > 2);
    storage_init(argv[--argc]);
    nufs_init_ops(&nufs_ops);
    return fuse_main(argc, argv, &nufs_ops, NULL);
}


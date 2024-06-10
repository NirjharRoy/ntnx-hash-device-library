/* Nirjhar Roy <babuiroy02@gmail.com> */

/*
 *  ntnx_hash.h - the header file with the ioctl definitions and data structures
 * used in libntnxhashlib.a library. These definitions are used both in the kernel
 * module as well as the libntnxhashlib.a library and the test/app that links
 * to the static library.
 */

#ifndef NTNX_HASH_H
#define NTNX_HASH_H

#include <linux/ioctl.h>
#include <stddef.h>

#define MAJOR_NUM 100
#define NTNX_HASH_GET_API_VERSION 0
#define NTNX_HASH_COMPUTE 1

/*
 * Gets the api version number
 */
#define IOCTL_NTNX_HASH_GET_API_VERSION _IOR(MAJOR_NUM, NTNX_HASH_GET_API_VERSION, char *)

/*
 * This returns the md5 hash
 */
#define IOCTL_NTNX_HASH_COMPUTE _IOR(MAJOR_NUM, NTNX_HASH_COMPUTE, char *)

/*
 * The name of the device file
 */
#define DEVICE_FILE_NAME "/dev/ntnx_hash"

/* For now (according the requirements) we are returning only a hardcoded version
 * number
 */
#define SUPPORTED_API_VERSION 1

struct ntnx_hash_compute {
    void *buf; // buffer which is to be hashed
    size_t len; // length of buf
    void *hash; // this will store the 32 character md5 hash
};

typedef struct ntnx_hash {
    int fd;
} ntnx_hash_t;

/* Wrapper functions exposed to the external application */
int ntnx_hash_destroy(ntnx_hash_t *ctx);
ntnx_hash_t *ntnx_hash_setup(void);
char* ntnx_hash_compute(ntnx_hash_t *ctx, void *buf, size_t len);

#endif
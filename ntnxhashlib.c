/* Nirjhar Roy <babuiroy02@gmail.com> */

/* ntnxhashlib.c - This file contains the code for the libntnxhashlib.a static
 * library. This basically contains wrapper functions that make the ioctl calls.
 * These wrapper functions will get called by the applications that links with
 * it.
 */
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "ntnx_hash.h"

/*
 * Functions for the ioctl calls
 */

static int get_api_version(ntnx_hash_t *ctx)
{
    int ret_val;
    int version;
	ret_val = ioctl(ctx->fd, IOCTL_NTNX_HASH_GET_API_VERSION, &version);

	if (ret_val < 0) {
		printf("get_api_version failed:%d\n", errno);
		exit(-1);
	}
    return version;
}

char* ntnx_hash_compute(ntnx_hash_t *ctx, void *buf, size_t len)
{
    if (!buf || len == 0) {
        return NULL;
    }

    void *hash_buf = malloc(len + 1);
    memset(hash_buf, 0, len + 1);
    assert(hash_buf);

    struct ntnx_hash_compute context = {
        .buf = buf,
        .len = len,
        .hash = hash_buf
    };

    int ret_val = ioctl(ctx->fd, IOCTL_NTNX_HASH_COMPUTE, &context);

	if (ret_val < 0) {
		printf("compute_hash failed: %d\n", errno);
		exit(-1);
	}

    return (char *)hash_buf;

}

ntnx_hash_t *ntnx_hash_setup(void)
{
    ntnx_hash_t *new_ctx = (ntnx_hash_t *)malloc(sizeof(*new_ctx));

    if (new_ctx) {
        int file_desc = open(DEVICE_FILE_NAME, 0);
        if (file_desc < 0) {
            printf("Can't open device file: %s errno = %d \n", DEVICE_FILE_NAME,
                errno);
            goto out_exit;
        }
        new_ctx->fd = file_desc;

        int cur_api_version = get_api_version(new_ctx);
        if (cur_api_version != SUPPORTED_API_VERSION) {
            printf("Unsupported api vesion. Expected %d got %d",
                    SUPPORTED_API_VERSION, cur_api_version);
            goto out_exit;
        }
    }
    return new_ctx;

out_exit:
    if (new_ctx) {
        free(new_ctx);
        exit(-1);
    }
}

int ntnx_hash_destroy(ntnx_hash_t *ctx)
{
    if (ctx) {
        return close(ctx->fd);
    }
    return 0;
}

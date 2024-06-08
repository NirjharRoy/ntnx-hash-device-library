/* Nirjhar Roy <babuiroy02@gmail.com> */

/* ntnx_hash_test.c - This file contains the test code to test the functionalities
 * of the ioctl calls and the ntnx md5 driver implementation. The way this test
 * works is as follows:-
 * 1) It takes an input string and calculates the md5 hash using md5sum tool.
 *    We call this expected_hash
 * 2) Then this test uses the functions in libntnxhashlib.a to caclulate the
 *    hash with the md5 device driver (we call this obtained_hash).
 * 3) Then it checks if the 2 hashes match (i.e, if expected_hash == obtained_hash)
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ntnx_hash.h"

#define TEST_MESSAGE "Message passed by ioctl"
#define HASH_LEN 32
char *MD5_SHELL_COMM = "echo -n \"" TEST_MESSAGE "\" | md5sum";

char* get_expected_hash(void)
{
    char *hash_to_match = calloc(1, HASH_LEN + 1);
    if (hash_to_match) {
        FILE *fp = NULL;
        int status;
        fp = popen(MD5_SHELL_COMM, "r");
        if (fp == NULL) {
            printf("Hash verification using md5sum failed. Try verifying manually\n");
            return NULL;
        }
        char *res = fgets(hash_to_match, HASH_LEN + 1, fp);
        hash_to_match[HASH_LEN] = '\0';

        if (res != NULL) {
            return hash_to_match;
        } else {
            printf("Couldn't read from md5sum command. Please check manually\n");
            return NULL;
        }
    } else {
        printf("Verification failed due to lack of memory. Please check manually\n");
        return NULL;
    }
}
int test(void)
{
    printf("\n\nTEST RUNNING\n");
    char *hash_obtained = NULL;
    char *hash_expected = NULL;
	char *msg = TEST_MESSAGE;

    /* Get the expected hash from the md5sum utility. If it is not installed
     * then we need to verify manually.
     */
    hash_expected = get_expected_hash();
    if (hash_expected) {
        printf("expected Hash of \"%s\" is \"%s\"\n", msg, hash_expected);
    } else {
        printf("Couln't get the expected hash. Please verify manually\n");
    }

    /* Get the hash by using ntnx_hash library */
    ntnx_hash_t *ctx = ntnx_hash_setup();
    if (!ctx) {
        printf("Context creation failed\n");
        goto out;
    }
    hash_obtained = ntnx_hash_compute(ctx, msg, strlen(msg));

    if (!hash_obtained) {
        printf("Hashing with ntnx library has failed.\n");
        goto out;
    }

    printf("Obtained Hash of \"%s\" is \"%s\"\n", msg, hash_obtained);

    /* If we were able to get the expected hash using the md5sum utility,
     * then we verify it.
     */
    if (hash_expected) {
        if(strncmp(hash_obtained, hash_expected, HASH_LEN)) {
            printf("!!!Hash mismatch!!!\n");
        } else {
            printf("Hashes matched. TEST PASSED\n");
        }
    } else {
        printf("Please verify hashes manually\n");
    }
out:
    if (ctx) {
	    assert(ntnx_hash_destroy(ctx) >= 0);
    }
    if (hash_obtained) {
        free(hash_obtained);
    }
    if(hash_expected) {
        free(hash_expected);
    }
    return 0;
}
int main(void)
{
    return test();
}
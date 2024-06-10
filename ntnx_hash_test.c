/* Nirjhar Roy <babuiroy02@gmail.com> */

/* ntnx_hash_test.c - This file contains the test code to test the functionalities
 * of the ioctl calls and the ntnx md5 driver implementation. The way this test
 * works is as follows:-
 * This test creates n processes. Each process spawns certain number of threads.
 * Threads within the same process share the same ntnx_hash_t context but
 * the different processes don't. Each thread does the following:
 * 1) It takes an input string(a randomly generated string) and calculates the
 *    md5 hash using md5sum tool. We call this expected_hash
 * 2) Then this test uses the functions in libntnxhashlib.a to caclulate the
 *    hash with the md5 device driver (we call this obtained_hash).
 * 3) Then it checks if the 2 hashes match (i.e, if expected_hash == obtained_hash)
 *
 * With this test we can verify that multiple threads (within a process) can share
 * the context and generate the hashes properly.
 * With multiple processes, we ensure that if multiple processes with different
 * context try to use the md5 device driver, it will still work with appropriate
 * synchronization mechanisms.
 */

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#include "ntnx_hash.h"

#define HASH_LEN 32
#define NELEM(a) (sizeof((a)) / sizeof((*a)))

/* The following parameters are configurable
 * ToDO: Make the following configurable from the command line
 */
#define MAX_NUM_THREADS 8
#define NUM_PROCESSES 4
#define MSG_SIZE (1 << 10)


typedef struct thread_params {
    int thread_index;
    ntnx_hash_t *ctx;
    char *msg;
} thread_params_t;

char* get_expected_hash(char *src)
{
    char *hash_to_match = calloc(1, HASH_LEN + 1);
    if (hash_to_match) {

        /* Generating the md5sum command to be run in the shell */
        int comm_len = strlen("echo -n \"") + strlen(src) + strlen("\" | md5sum");
        char *command = malloc((comm_len + 2)* sizeof(*command));
        assert(command);
        snprintf(command, comm_len + 1, "%s%s%s", "echo -n \"", src,  "\" | md5sum");

        FILE *fp = NULL;
        int status;
        fp = popen(command, "r");
        free(command);
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
void* test_per_thread(void *param)
{
    thread_params_t *params = (thread_params_t *)param;
    char *hash_obtained = NULL;
    char *hash_expected = NULL;
	char *msg = (char *)malloc(MSG_SIZE * sizeof(*msg));
    assert(msg);

    for (int i = 0; i < MSG_SIZE - 1; i++) {
        if (i % 2) {
            msg[i] = 'a' + rand() % 26;
        } else {
            msg[i] = 'A' + rand() % 26;
        }
    }
    msg[MSG_SIZE - 1] = '\0';

    ntnx_hash_t *ctx = params->ctx;

    /* Get the expected hash from the md5sum utility. If it is not installed
     * then we need to verify manually.
     */
    hash_expected = get_expected_hash(msg);
    if (hash_expected) {
        //printf("expected Hash of \"%s\" is \"%s\"\n", msg, hash_expected);
    } else {
        printf("Couln't get the expected hash. Please verify manually\n");
    }

    /* Get the hash by using ntnx_hash library */

    hash_obtained = ntnx_hash_compute(ctx, msg, strlen(msg));

    if (!hash_obtained) {
        printf("Hashing with ntnx library has failed.\n");
        goto out;
    }

    //printf("Obtained Hash of \"%s\" is \"%s\"\n", msg, hash_obtained);

    /* If we were able to get the expected hash using the md5sum utility,
     * then we verify it.
     */
    if (hash_expected) {
        if(strncmp(hash_obtained, hash_expected, HASH_LEN)) {
            printf("!!!Hash mismatch!!! for thread index = %d pid = %d\n\n",
                    params->thread_index, getpid());
            exit(-1);
        } else {
            printf("Hashes matched. TEST PASSED for thread index %d pid = %d\n\n",
                   params->thread_index, getpid());
        }
    } else {
        printf("Please verify hashes manually\n");
    }
out:
    if (hash_obtained) {
        free(hash_obtained);
    }
    if (hash_expected) {
        free(hash_expected);
    }
    if (msg) {
        free(msg);
    }
}

int test_per_process(void)
{
    printf("\n STARTING TEST \n");
    srand(time(0));
    int ret = 0;
    ntnx_hash_t *ctx = ntnx_hash_setup();
    if (!ctx) {
        fprintf(stderr, "Error creating context\n");
        ret = -1;
        goto out;
    }

    pthread_t threads[MAX_NUM_THREADS];
    for (int i = 0; i < NELEM(threads); ++i) {
        // Allocate memory for thread parameters
        thread_params_t* param = (thread_params_t*)malloc(sizeof(*param));
        if (param == NULL) {
            fprintf(stderr, "Error allocating memory for pthread\n");
            ret = -1;
            goto out;
        }
        param->thread_index = i;
        param->ctx = ctx;

        // Create a new thread
        int ret = pthread_create(&threads[i], NULL, test_per_thread, (void*)param);
        if (ret) {
            fprintf(stderr, "Error creating thread: %d with errno = %d\n", i, errno);
            ret = -1;
            goto out;
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < NELEM(threads); ++i) {
        pthread_join(threads[i], NULL);
    }
    printf("ALL TEST PASSED!!\n");
out:
    if (ctx) {
	    assert(ntnx_hash_destroy(ctx) >= 0);
    }
    if (ret) {
        exit(EXIT_FAILURE);
    }
    return 0;

}

void test(void)
{
    pid_t pid;
    int status;
    int num_children = NUM_PROCESSES; // Number of child processes to create

    for (int i = 0; i < num_children; i++) {
        pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        } else if (pid == 0) {
            printf("Child %d (PID: %d) started\n", i, getpid());
            test_per_process();
            printf("Child %d (PID: %d) exiting\n", i, getpid());
            exit(0); // Exit child process
        }
    }

    // Parent process
    for (int i = 0; i < num_children; i++) {
        pid_t child_pid = wait(&status);

        if (child_pid > 0) {
            if (WIFEXITED(status)) {
                printf("Parent: Child %d (PID: %d) exited with status %d\n",
                       i, child_pid, WEXITSTATUS(status));
            } else {
                printf("Parent: Child %d (PID: %d) did not exit successfully\n",
                        i, child_pid);
            }
        } else {
            perror("Wait failed");
            exit(1);
        }
    }
    printf("\n!!!!! All tests passed for all the processes and their child threads !!!!!!\n");
}

int main(void)
{
    test();
    return 0;
}
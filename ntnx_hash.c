/* Nirjhar Roy <babuiroy02@gmail.com> */

/*
 * ntnx_hash.c - This file contains the code for the ntnx md5 hash driver.
 * It contains the logic for md5 hashing as well the implementations for the
 * ioctl calls.
 */
#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "ntnx_hash.h"

#define SUCCESS 0
#define DEVICE_NAME "ntnx_hash"
#define BUF_LEN 80
#define API_VERSION SUPPORTED_API_VERSION

/*
 * Is the device open right now? Used to prevent
 * concurent access into the same device
 */
static int Device_Open = 0;

/*
 * The message the device will give when asked
 */

struct mychar_device_data {
    struct cdev cdev;
};

static int dev_major = 0;
static struct class *mychardev_class = NULL;
static struct mychar_device_data mychardev_data;



/* MD5 related functions */
double sin_arr[64] = { 0.841470984807897, 0.909297426825682, 0.141120008059867,
                       0.756802495307928, 0.958924274663138, 0.279415498198926,
                       0.656986598718789, 0.989358246623382, 0.412118485241757,
                       0.544021110889370, 0.999990206550703, 0.536572918000435,
                       0.420167036826641, 0.990607355694870, 0.650287840157117,
                       0.287903316665065, 0.961397491879557, 0.750987246771676,
                       0.149877209662952, 0.912945250727628, 0.836655638536056,
                       0.008851309290404, 0.846220404175171, 0.905578362006624,
                       0.132351750097773, 0.762558450479603, 0.956375928404503,
                       0.270905788307869, 0.663633884212968, 0.988031624092862,
                       0.404037645323065, 0.551426681241691, 0.999911860107267,
                       0.529082686120024, 0.428182669496151, 0.991778853443116,
                       0.643538133356999, 0.296368578709385, 0.963795386284088,
                       0.745113160479349, 0.158622668804709, 0.916521547915634,
                       0.831774742628598, 0.017701925105414, 0.850903524534118,
                       0.901788347648809, 0.123573122745224, 0.768254661323667,
                       0.953752652759472, 0.262374853703929, 0.670229175843375,
                       0.986627592040485, 0.395925150181834, 0.558789048851616,
                       0.999755173358620, 0.521551002086912, 0.436164755247825,
                       0.992872648084537, 0.636738007139138, 0.304810621102217,
                       0.966117770008393, 0.739180696649223, 0.167355700302807,
                       0.920026038196791
                    };


typedef union uwb {
	unsigned w;
	unsigned char b[4];
} MD5union;

typedef unsigned DigestArray[4];

unsigned func0(unsigned abcd[]) {
	return (abcd[1] & abcd[2]) | (~abcd[1] & abcd[3]);
}

unsigned func1(unsigned abcd[]) {
	return (abcd[3] & abcd[1]) | (~abcd[3] & abcd[2]);
}

unsigned func2(unsigned abcd[]) {
	return  abcd[1] ^ abcd[2] ^ abcd[3];
}

unsigned func3(unsigned abcd[]) {
	return abcd[2] ^ (abcd[1] | ~abcd[3]);
}

typedef unsigned(*DgstFctn)(unsigned a[]);

unsigned *calctable(unsigned *k)
{
	double s, pwr;
	int i;
	pwr = (double)( (long)1 << 32);
	for (i = 0; i < 64; i++) {
		s = sin_arr[i];
		k[i] = (unsigned)(s * pwr);
	}
	return k;
}

unsigned rol(unsigned r, short N)
{
	unsigned  mask1 = (1 << N) - 1;
	return ((r >> (32 - N)) & mask1) | ((r << N) & ~mask1);
}

unsigned* Algorithms_Hash_MD5(const char *msg, int mlen)
{
	static DigestArray h0 = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476 };
	static DgstFctn ff[] = { &func0, &func1, &func2, &func3 };
	static short M[] = { 1, 5, 3, 7 };
	static short O[] = { 0, 1, 5, 0 };
	static short rot0[] = { 7, 12, 17, 22 };
	static short rot1[] = { 5, 9, 14, 20 };
	static short rot2[] = { 4, 11, 16, 23 };
	static short rot3[] = { 6, 10, 15, 21 };
	static short *rots[] = { rot0, rot1, rot2, rot3 };
	static unsigned kspace[64];
	static unsigned *k;

	static DigestArray h;
	DigestArray abcd;
	DgstFctn fctn;
	short m, o, g;
	unsigned f;
	short *rotn;
	union {
		unsigned w[16];
		char     b[64];
	}mm;
	int os = 0;
	int grp, grps, q, p;
	unsigned char *msg2;

	if (k == NULL) k = calctable(kspace);

	for (q = 0; q<4; q++) h[q] = h0[q];

	{
		grps = 1 + (mlen + 8) / 64;
		msg2 = (unsigned char*)kzalloc(64 * grps, GFP_KERNEL);
		memcpy(msg2, msg, mlen);
		msg2[mlen] = (unsigned char)0x80;
		q = mlen + 1;
		while (q < 64 * grps) { msg2[q] = 0; q++; }
		{
			MD5union u;
			u.w = 8 * mlen;
			q -= 8;
			memcpy(msg2 + q, &u.w, 4);
		}
	}

	for (grp = 0; grp<grps; grp++)
	{
		memcpy(mm.b, msg2 + os, 64);
		for (q = 0; q<4; q++) abcd[q] = h[q];
		for (p = 0; p<4; p++) {
			fctn = ff[p];
			rotn = rots[p];
			m = M[p]; o = O[p];
			for (q = 0; q<16; q++) {
				g = (m*q + o) % 16;
				f = abcd[1] + rol(abcd[0] + fctn(abcd) + k[q + 16 * p] + mm.w[g], rotn[q % 4]);

				abcd[0] = abcd[3];
				abcd[3] = abcd[2];
				abcd[2] = abcd[1];
				abcd[1] = f;
			}
		}
		for (p = 0; p<4; p++)
			h[p] += abcd[p];
		os += 64;
	}
	kfree(msg2);
	return h;
}

void GetMD5String(const char *msg, int mlen, char *hash) {

	int j;
	//int k;
	unsigned *d = Algorithms_Hash_MD5(msg, mlen);
	char s[8];
	MD5union u;
	for (j = 0; j < 4; j++) {
		u.w = d[j];
		sprintf(s, "%02x%02x%02x%02x", u.b[0], u.b[1], u.b[2], u.b[3]);
		strcat(hash, s);
		hash += 4;
	}
}
/* MD5 related functions ends */


/* driver specific functions begin */
static int get_api_version(void)
{
	return API_VERSION;
}

/*
 * This is called whenever a  process attempts to open the device file
 */
static int device_open(struct inode *inode, struct file *file)
{
#ifdef DEBUG
	printk(KERN_INFO "device_open(%p)\n", file);
#endif

	/*
	 * We don't want to talk to two processes at the same time
	 */
	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	try_module_get(THIS_MODULE);
	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
#ifdef DEBUG
	printk(KERN_INFO "device_release(%p,%p)\n", inode, file);
#endif

	/*
	 * We're now ready for our next caller
	 */
	Device_Open--;

	module_put(THIS_MODULE);
	return SUCCESS;
}

static ssize_t
compute_md5_hash(struct ntnx_hash_compute *data)
{
	size_t len = data->len;
    ssize_t ret = SUCCESS;
    char hash[33] = {0, };
	char *src_kernel = NULL;

    src_kernel = (char *)kzalloc(len, GFP_KERNEL);
	if (!src_kernel) {
		ret = -ENOMEM;
		goto out;
	}

	if (copy_from_user(src_kernel, data->buf, len)) {
		ret = -EFAULT;
		goto out;
	}

	GetMD5String(src_kernel, len, hash);

	hash[32] = '\0';

	if (copy_to_user(data->hash, hash, sizeof(hash))) {
		ret = -EFAULT;
		goto out;
	}

out:
    if (src_kernel) {
		kfree(src_kernel);
	}
	return ret;
}


long device_ioctl(struct file *file,
		          unsigned int ioctl_num,
		          unsigned long ioctl_param)
{
	int version;
	struct ntnx_hash_compute hash_data;
    ssize_t ret;

	switch (ioctl_num) {
	case IOCTL_NTNX_HASH_GET_API_VERSION:
		version = get_api_version();
        printk(KERN_INFO "IOCTL_NTNX_HASH_GET_API_VERSION CALLED");
		if (copy_to_user((void *)ioctl_param, &version, sizeof(version))) {
			return -EFAULT;
		}
		break;

	case IOCTL_NTNX_HASH_COMPUTE:

		if (copy_from_user((void *)&hash_data, (void *)ioctl_param, sizeof(hash_data))) {
			return -EFAULT;
		}

		ret = compute_md5_hash(&hash_data);
		return ret;
    default:
	    return -ENOSYS;
	}
	return SUCCESS;
}

struct file_operations Fops = {
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_release,	/* a.k.a. close */

};

static int mychardev_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

int init_module()
{

    dev_major = MAJOR_NUM;
    mychardev_class = class_create(THIS_MODULE, DEVICE_NAME);
    mychardev_class->dev_uevent = mychardev_uevent;

	cdev_init(&mychardev_data.cdev, &Fops);
	mychardev_data.cdev.owner = THIS_MODULE;
	cdev_add(&mychardev_data.cdev, MKDEV(dev_major, 0), 1);
	device_create(mychardev_class, NULL, MKDEV(dev_major, 0), NULL, DEVICE_NAME);

    return 0;
}

void cleanup_module()
{
    device_destroy(mychardev_class, MKDEV(MAJOR_NUM, 0));
    class_unregister(mychardev_class);
    class_destroy(mychardev_class);
    unregister_chrdev_region(MKDEV(MAJOR_NUM, 0), MINORMASK);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nirjhar Roy <babuiroy02@gmail.com>");
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mariama A");
MODULE_DESCRIPTION("My linux kernel module");
MODULE_VERSION("0.01");

#define DEVICE_NAME "mymod"
#define EXAMPLE_MSG "My example!\n"
#define MSG_BUFFER_LEN 13

/*
Device functions
*/
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static int major_num = 0;
static int device_open_count = 0;
static char msg_buffer[MSG_BUFFER_LEN];
static char *msg_ptr = NULL;

/*
Pointer to device functions
*/
static struct file_operations file_ops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
	};

static ssize_t device_read(struct file *flip, char *buffer, size_t len, loff_t *offset){
	int bytes_read = 0;
	if (msg_ptr == NULL){
		msg_ptr = msg_buffer;
	}

	while(len && *msg_ptr){
		put_user(*(msg_ptr++), buffer++);
		len--;
		bytes_read++;
	}
	return bytes_read;
}

static ssize_t device_write(struct file *flip, const char *buffer, size_t len, loff_t *offset){
	printk(KERN_ALERT "Read only device");
	return -EINVAL;
}

static int device_open(struct inode *inode, struct file *file){
	//Check if device is already open
	if(device_open_count){
		return -EBUSY;
	}

	device_open_count++;
	try_module_get(THIS_MODULE);
	return 0;
}

/*Called when device is closed*/
static int device_release(struct inode *inode, struct file *file){
	/*Update device reference counter*/
	device_open_count--;
	module_put(THIS_MODULE);
	return 0;
}

static int __init mymod_init(void){
	printk(KERN_INFO "Hello My kernel\n");
	//Make sure string buffer ends in '\0'
	msg_buffer[MSG_BUFFER_LEN] = '\0';

	//Put message in buffer
	strncpy(msg_buffer, EXAMPLE_MSG, MSG_BUFFER_LEN);
	msg_ptr = msg_buffer;

	/*Register character device*/
	major_num = register_chrdev(0, "mymod", &file_ops);
	if (major_num < 0){
		//Failed to regiser device
		printk(KERN_ALERT "Failed to register device %d.\n", major_num);
	} else {
		printk(KERN_INFO "mymod module loaded with device major number %d\n", major_num);
		return 0;
	}
	return 0;
}

static void __exit mymod_exit(void){
	printk(KERN_INFO "Goodbye My kernel\n");
	unregister_chrdev(major_num, DEVICE_NAME);
	printk(KERN_INFO "Goodbye, World!\n");
}

module_init(mymod_init);
module_exit(mymod_exit);

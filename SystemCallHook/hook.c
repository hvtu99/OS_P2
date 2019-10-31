
#include <asm/unistd.h>
#include <asm/cacheflush.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <asm/pgtable_types.h>
#include <linux/highmem.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>
#include <asm/cacheflush.h>
#include <linux/fcntl.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huynh Van Tu - Nguyen Xuan Vy");
MODULE_DESCRIPTION("Hook system call open and write");



#define GPF_DISABLE write_cr0(read_cr0() & (~ 0x10000))
#define GPF_ENABLE write_cr0(read_cr0() | 0x10000)

/*Make page writeable*/
int make_rw(unsigned long address){
    unsigned int level;
    pte_t *pte = lookup_address(address, &level);
    if(pte->pte &~_PAGE_RW){
        pte->pte |=_PAGE_RW;
    }
    return 0;
}
/* Make the page write protected */
int make_ro(unsigned long address){
    unsigned int level;
    pte_t *pte = lookup_address(address, &level);
    pte->pte = pte->pte &~_PAGE_RW;
    return 0;
}




unsigned long **sys_call_table;

asmlinkage int (*original_sys_open)(const char __user *, int, mode_t);
asmlinkage size_t (*original_sys_write)(unsigned int, const char __user *, size_t);


/*hook*/
asmlinkage int new_sys_open(const char __user *pathname, int flags, mode_t mode)
{
	printk(KERN_INFO "HOOK open syscall\n");
	printk(KERN_INFO "Process name: %s\n", current->comm);
	printk(KERN_INFO "File name: %s\n", pathname);

	return (*original_sys_open)(pathname, flags, mode);
}

asmlinkage size_t new_sys_write(unsigned int fd, const char __user *buf, size_t count)
{
	size_t bytes_write;

	printk(KERN_INFO "HOOK write syscall\n");
	printk(KERN_INFO "Process name: %s\n", current->comm);

	bytes_write = (*original_sys_write)(fd, buf, count);
	printk(KERN_INFO "Bytes write: %d\n, ", bytes_write);

	return bytes_write;
}




static void get_syscall_table(void)
{
	unsigned long int offset = PAGE_OFFSET;

	while (offset < ULLONG_MAX) {
		unsigned long **temp_sys_call_table = (unsigned long **)offset;
		
		if (temp_sys_call_table[__NR_close] ==
		    (unsigned long *)sys_close) {
			sys_call_table = temp_sys_call_table;
			return;
		}

		offset += sizeof(void *);
	}
	syscall_table = NULL;
}


static int __init init_hook(void){
    	printk(KERN_INFO "Captain Hook loaded successfully..\n");
   	get_syscall_table();

	if (syscall_table == NULL) {
		printk(KERN_ERR "HOOK not found syscall table\n");
		return -1;
	}

	original_sys_open = (void *)sys_call_table[__NR_open];
	original_sys_write = (void *)sys_call_table[__NR_write];

	//GPF_ENABLE();
	make_rw(sys_tem_table);

	sys_call_table[__NR_open] = (unsigned long *)new_sys_open;
	sys_call_table[__NR_write] = (unsigned long *)new_sys_write;

	//GPF_DISABLE();
	return 0;
}

static void __exit exit_hook(void){
    	printk(KERN_INFO "Unloaded Captain Hook successfully\n");
    
    	if (sys_call_table != NULL) {
		//GPF_ENABLE;		

		sys_call_table[__NR_open] = (unsigned long *)original_sys_open;
		sys_call_table[__NR_write] = (unsigned long *)original_sys_write;
		make_ro(sys_tem_table);
		//GPF_DISABLE;
	}

}
module_init(init_hook);
module_exit(exit_hook);


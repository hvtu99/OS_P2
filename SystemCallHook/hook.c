
#include <asm/unistd.h>
#include <asm/cacheflush.h>
#include <asm/pgtable_types.h>
#include <asm/cacheflush.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/highmem.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>
#include <linux/fcntl.h>
#include <linux/fdtable.h>
#include <linux/slab.h>
#include <linux/kallsyms.h>




MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huynh Van Tu - Nguyen Xuan Vy");
MODULE_DESCRIPTION("Hook system call open and write");


#define ENABLE_WRITE write_cr0(read_cr0() & (~0x10000))
#define DISABLE_WRITE write_cr0(read_cr0() | 0x10000)


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


unsigned long **sys_call_table_addr;

asmlinkage int (*original_sys_open)(const char __user *, int, mode_t);
asmlinkage int (*original_sys_write)(unsigned int, const char __user *, size_t);


/*hook*/
asmlinkage int new_sys_open(const char __user *pathname, int flags, mode_t mode)
{
	printk(KERN_INFO "HooK open syscall\n");
	printk(KERN_INFO "Process name: %s\n", current->comm);
	printk(KERN_INFO "File name: %s\n", pathname);

	return (*original_sys_open)(pathname, flags, mode);
}

asmlinkage int new_sys_write(unsigned int fd, const char __user *buf, size_t count)
{
	int bytes_write;
	char *buffer   = kmalloc(1024, GFP_KERNEL);
	char *pathname   = d_path(&fcheck_files(current->files, fd)->f_path, buffer, 1024);

	printk(KERN_INFO "HooK write syscall\n");
	printk(KERN_INFO "Process name: %s\n", current->comm);
	printk(KERN_INFO "File name: %s\n", pathname);	
	bytes_write = (*original_sys_write)(fd, buf, count);
	printk(KERN_INFO "Bytes write: %d\n, ", bytes_write);
	
	kfree(buffer);
	return bytes_write;
}



/*
static void get_sys_call_table_addr(void)
{
	unsigned long int offset = PAGE_OFFSET;

	while (offset < ULLONG_MAX) {
		unsigned long **temp_sys_call_table_addr = (unsigned long **)offset;
		
		if (temp_sys_call_table_addr[__NR_close] ==
		    (unsigned long *)ksys_close) {
			sys_call_table_addr = temp_sys_call_table_addr;
			return;
		}

		offset += sizeof(void *);
	}
	sys_call_table_addr = NULL;
}
*/

static int __init init_hook(void){
    	printk(KERN_INFO "Hook loaded successfully..\n");
   	sys_call_table_addr = (unsigned long **) kallsyms_lookup_name("sys_call_table");
	//get_sys_call_table_addr();

	if (sys_call_table_addr == NULL) {
		printk(KERN_ERR "HooK not found syscall table\n");
		return -1;
	}

	original_sys_open = (void *)sys_call_table_addr[__NR_open];
	original_sys_write = (void *)sys_call_table_addr[__NR_write];

	//ENABLE_WRITE;
	make_rw((unsigned long)sys_call_table_addr);

	sys_call_table_addr[__NR_open] = (unsigned long *)new_sys_open;
	sys_call_table_addr[__NR_write] = (unsigned long *)new_sys_write;
	
	//DISABLE_WRITE;
	make_ro((unsigned long)sys_call_table_addr);
	return 0;
}

static void __exit exit_hook(void){
    	printk(KERN_INFO "Unloaded Hook successfully\n");
    
    	if (sys_call_table_addr != NULL) {
		//ENABLE_WRITE;	
		make_rw((unsigned long)sys_call_table_addr);
		
		sys_call_table_addr[__NR_open] = (unsigned long *)original_sys_open;
		sys_call_table_addr[__NR_write] = (unsigned long *)original_sys_write;
		
		//DISABLE_WRITE;
		make_ro((unsigned long)sys_call_table_addr);
	}

}
module_init(init_hook);
module_exit(exit_hook);


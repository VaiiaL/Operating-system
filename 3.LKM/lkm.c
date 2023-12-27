#include <linux/module.h>
#include <linux/printk.h> /* Needed for pr_info() */ 

MODULE_LICENSE("GPL");

static int __init lkm_init(void) 
{
    pr_info("Welcome to the Tomsk State University!\n");
    return 0;
}

static void __exit lkm_exit(void) 
{
    pr_info("Tomsk State University forever!\n");
}

module_init(lkm_init);
module_exit(lkm_exit);
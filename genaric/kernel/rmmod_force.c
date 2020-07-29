#include <linux/module.h>
#include <linux/init.h>
#include <asm-generic/local.h>

static char *modname = "netlinkKernel";
module_param(modname, charp, 0644);
MODULE_PARM_DESC(modname, "The name of module you wanna clean.\n");

static int __init rmmod_force_init(void)
{
        struct module *mod = NULL;
        int cpu;
        if ((mod = find_module(modname)) == NULL)
                printk(KERN_ALERT "This [%s] not found!\n", modname);
        mod->state = MODULE_STATE_LIVE;
        // 设置每一个 CPU 上缓存的 mod->refcnt 为 0
        for_each_possible_cpu(cpu)
                local_set((local_t*)per_cpu_ptr(&(mod->refcnt), cpu), 0);
       // 设置 mod->refcnt 变量值 1
        atomic_set(&mod->refcnt, 1);
        return 0;
}

static void __exit rmmod_force_exit(void)
{
        printk(KERN_ALERT "Bye man, I have been unload.\n");
}

module_init(rmmod_force_init);
module_exit(rmmod_force_exit);
MODULE_AUTHOR("Jackie Liu <liuyun01@kylinos.cn>");
MODULE_LICENSE("GPL");

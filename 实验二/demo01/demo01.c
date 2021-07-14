#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/sched/signal.h>

static int show_all_process_init(void) {
 	struct task_struct *p;
	p=NULL;
	printk(KERN_ALERT"Demo01 Init");
	printk("              进程名    PID   状态 优先级 父进程\n");
	for_each_process(p) {
	        if(p->mm==NULL) {
    			printk("%20s %6d %6ld %6d %6d\n",p->comm,p->pid,p->state,p->prio,p->parent->pid);
       		}
    	}
	return 0;
}
static void show_all_process_exit(void) {
	printk(KERN_ALERT"Demo01 Exit\n");
}
module_init(show_all_process_init);
module_exit(show_all_process_exit);

MODULE_LICENSE("GPL");

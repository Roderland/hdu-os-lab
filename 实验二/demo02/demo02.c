#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/pid.h>

#include<linux/types.h>
#include<linux/sched.h>
#include<linux/list.h>
#include<linux/moduleparam.h>

static int pid;
module_param(pid, int, 0644);

static int show_family_init(void) {

    struct task_struct *task;
    struct task_struct *parent;

    struct list_head *p_sibling;
    struct task_struct *sibling_task;

    struct list_head *p_children;
    struct task_struct *children_task;

    task = pid_task(find_get_pid(pid), PIDTYPE_PID);
    parent = task->parent;

    printk(KERN_ALERT"Demo02 Init");

    printk("%-20s %-5d %-5ld\n", parent->comm, parent->pid, parent->state);

    list_for_each(p_sibling, &(parent->children)) {

            sibling_task = list_entry(p_sibling, struct task_struct, sibling);

	    if(p_sibling->next==&(parent->children))
	    {
    		    printk("   └───%-20s %-5d %-5ld\n", sibling_task->comm, sibling_task->pid, sibling_task->state);

		    list_for_each(p_children, &(sibling_task->children)) {

	    		    children_task = list_entry(p_children, struct task_struct, sibling);

			    if(p_children->next==&(sibling_task->children))
				    printk("          └───%-20s %-5d %-5ld\n", children_task->comm, children_task->pid, children_task->state);
			    else
				    printk("          ├───%-20s %-5d %-5ld\n", children_task->comm, children_task->pid, children_task->state);
		    }
	    }
	    else
	    {
    		    printk("   ├───%-20s %-5d %-5ld\n", sibling_task->comm, sibling_task->pid, sibling_task->state);
		   
		    list_for_each(p_children, &(sibling_task->children)) {
			   
			    children_task = list_entry(p_children, struct task_struct, sibling);

	    		    if(p_children->next==&(sibling_task->children))
			    	    printk("   │      └───%-20s %-5d %-5ld\n", children_task->comm, children_task->pid, children_task->state);
		    	    else
			    	    printk("   │      ├───%-20s %-5d %-5ld\n", children_task->comm, children_task->pid, children_task->state);
		    }
	    }
    }

    return 0;
}

static void show_family_exit(void) {
    printk(KERN_ALERT"Demo02 Exit\n");
}

module_init(show_family_init);
module_exit(show_family_exit);

MODULE_LICENSE("GPL");

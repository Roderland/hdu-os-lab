//增加的三个系统调用


336	64	show_sysname		__x64_sys_show_sysname
337	64	mod_hostname		__x64_sys_mod_hostname
338     64      set_nice	        __x64_sys_set_nice


asmlinkage long sys_show_sysname(void);
asmlinkage long sys_mod_hostname(char __user * name, int len, void __user*res);
asmlinkage long sys_set_nice(pid_t pid,int nicevalue,void __user*prio,void __user*nice);


SYSCALL_DEFINE0(show_sysname)
{
	printk("%s %s", UTS_SYSNAME, UTS_RELEASE);
	return 0;
}

SYSCALL_DEFINE3(mod_hostname, char __user *, name, int, len, void __user *, res)
{
	int errno;
	char tmp[__NEW_UTS_LEN];

	if (len < 0 || len > __NEW_UTS_LEN)
		return -EINVAL;

	errno = -EFAULT;
	if (!copy_from_user(tmp, name, len)) {
		struct new_utsname *u;
		down_write(&uts_sem);
		u = utsname();
		memcpy(u->nodename, tmp, len);
		memset(u->nodename + len, 0, sizeof(u->nodename) - len);
		copy_to_user(res, u->nodename, sizeof(u->nodename));
		errno = 0;
		uts_proc_notify(UTS_PROC_HOSTNAME);
		up_write(&uts_sem);
	}
	return errno;
}

SYSCALL_DEFINE4(set_nice, pid_t, pid, int, nicevalue, void __user *, prio,
		void __user *, nice)
{
	struct pid *kpid;
	struct task_struct *task;
	kpid = find_get_pid(pid);
	task = pid_task(kpid, PIDTYPE_PID);
	int n;
	n = task_nice(task);
	int p;
	p = task_prio(task);

	set_user_nice(task, nicevalue);
	n = task_nice(task);
	p = task_prio(task);
	copy_to_user(nice, &n, sizeof(n));
	copy_to_user(prio, &p, sizeof(p));
	return 0;
}


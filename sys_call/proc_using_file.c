#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/sched/mm.h>
#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/rcupdate.h>
#include <linux/cred.h>
#include <linux/namei.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <uapi/linux/proc_using_file.h>

static int match_inode(const void *p, struct file *file, unsigned int fd)
{
	const struct inode *target = p;
    return file_inode(file) == target ? 1 : 0;
}

SYSCALL_DEFINE3(proc_using_file,
        const char __user *, pathname,
        struct proc_info __user *, ubuf,
        int, max_entries)
{
	struct path path;
	struct inode *target;
	struct task_struct *task;
	struct task_struct **matched;
    	int count = 0, i, ret = 0;
    	char *kpath;

	if (!pathname || !ubuf || max_entries <= 0)
		return -EINVAL;

	kpath = strndup_user(pathname, PATH_MAX);
	if (IS_ERR(kpath))
		return PTR_ERR(kpath);

	ret = kern_path(kpath, LOOKUP_FOLLOW, &path);
	kfree(kpath);
	if (ret)
		return ret;

	target = path.dentry->d_inode;

	matched = kmalloc_array(max_entries, sizeof(*matched), GFP_KERNEL);
	if (!matched) {
		path_put(&path);
		return -ENOMEM;
	}

	rcu_read_lock();
	for_each_process(task) {
		int hit;

		if (count >= max_entries)
			break;

		task_lock(task);
		hit = task->files
			? iterate_fd(task->files, 0, match_inode, target)
			: 0;
		task_unlock(task);
		if (hit) {
			get_task_struct(task);
			matched[count++] = task;
		}
	}
	rcu_read_unlock();
	path_put(&path);


	for (i = 0; i < count; i++) {
		struct proc_info  info = {0};
		struct mm_struct *mm;
		task = matched[i];

		info.pid  = task_pid_nr(task);
		info.prio = task->prio;
		info.nice = task_nice(task);

		rcu_read_lock();
		info.uid = from_kuid(&init_user_ns, task_uid(task));
		rcu_read_unlock();

		mm = get_task_mm(task);
		if (mm) {
			info.mem_kb = get_mm_rss(mm) << (PAGE_SHIFT - 10);
			mmput(mm);
		}

		info.cpu_time_ns = task->utime + task->stime;

		get_task_comm(info.comm, task);

		if (copy_to_user(&ubuf[i], &info, sizeof(info)))
			ret = -EFAULT;

        	put_task_struct(task);
	}

	kfree(matched);
	return ret ? ret : count;
}
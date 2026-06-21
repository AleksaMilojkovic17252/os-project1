#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/rcupdate.h>
#include <linux/cred.h>
#include <linux/namei.h>
#include <linux/string.h>

static char *target = "";
module_param(target, charp, 0444);
MODULE_PARM_DESC(target, "absolute path of the file to inspect");

static int match_fd(const void *p, struct file *f, unsigned int fd)
{
  const struct inode *target = p;
  return file_inode(f) == target ? 1 : 0;
}

static int __init pu_init(void)
{
  struct path path;
  struct inode *tgt;
  struct task_struct *task;
  
  int ret;
  int found = 0;
  
  if(!target || !*target) {
    pr_err("procusers: no target specified.");
    return -EINVAL;
  }
  
  ret = kern_path(target, LOOKUP_FOLLOW, &path);
  if (ret) {
    pr_err("procusers: cannot resolve path");
    return ret;
  }
  
  tgt = d_inode(path.dentry);
  
  pr_info("procusers: scanning for processes");
  
  rcu_read_lock();
  for_each_process(task) {
    int hit;
    unsigned long kb = 0;
    
    task_lock(task);
    hit = task->files ? iterate_fd(task->files, 0, match_fd, tgt) : 0;
    if(hit && task->mm)
      kb = get_mm_rss(task->mm) << (PAGE_SHIFT - 10);
    task_unlock(task);
    
    if(hit)
    {
      pr_info("PID %-6d  prio %-3d  nice %-3d  uid %-6u mem %-8lu kB  cpu %llu ns  comm: %s\n", task_pid_nr(task), task->prio, task_nice(task), from_kuid(&init_user_ns, task_uid(task)), kb, task->utime + task->stime, task->comm);
      found++;
    }
  }
  //unlock
  rcu_read_unlock();
  if(found)
    pr_info("procusers: found %d process(es)\n", found);
  else
    pr_info("procusers: no processes found using '%s'\n", target);
    
  path_put(&path);
  return 0;
}

static void __exit pu_exit(void)
{
  pr_info("procusers: module unloaded\n");
}

module_init(pu_init);
module_exit(pu_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("List processes that have a given file open");
MODULE_AUTHOR("Aleksa");

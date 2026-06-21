#ifndef _UAPI_LINUX_PROC_USING_FILE_H
#define _UAPI_LINUX_PROC_USING_FILE_H

struct proc_info {
	int pid;
	int prio;
	int nice;
	unsigned int uid;
	unsigned long mem_kb;
	unsigned long long cpu_time_ns;
	char comm[16];
};

#endif
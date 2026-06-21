# Syscall part: Guide

After downloading the kernel and installing the new kernel using the following command:

```bash
    scripts/config --disable SYSTEM_TRUSTED_KEYS
    scripts/config --disable SYSTEM_REVOCATION_KEYS
    make -j$(nproc)
    sudo make modules_install
    sudo make install
```
_Make sure to make changes in grub to see the new kernel_

Make changes in the following files:
1. ~/linux-6.12/include/uapi/linux/proc_using_file.h -> Add this file for the proc_info struct
2. ~/linux-6.12/arch/x86/entry/syscalls/syscall_64.tbl -> Add the syscall to the table, it must be before the 32-bit calls.
3. ~/linux-6.12/include/linux/syscalls.h -> Add the call via asmlinkage
4. ~/linux-6.12/kernel/proc_using_file.c -> Create the SYSCALL
5. ~/linux-6.12/kernel/Makefile -> edit the Makefile to include the output of the syscall.

After that run the commands above again to install the new kerner with the new syscall

After the restart creat the following test file
- test_proc_using_file.c
This will call the syscall, build it with gcc and run the following commands

```bash
touch /tmp/testfile
tail -f /tmp/testfile &
```
So some process is reading the file

then run the syscall with the following command
```bash
./test_proc_using_file /tmp/testfile
```

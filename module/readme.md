# Module part: Guide

## Workspace
Create the following files:

- procusers.c
- Makefile

After creating the files, run the following command

```bash
make
```

## Run the module

Create the tmp file to test

```bash
touch /tmp/testfile
tail -f /tmp/testfile &
```

Include the module in the kernel:
```bash
sudo insmod procusers.ko target="/tmp/testfile"
```
After that run the dmesg command to read the output of the module
```bash
sudo dmesg | tail -n 20
```
In the end, remove the module to test it again
```bash
sudo rmmod procusers
```

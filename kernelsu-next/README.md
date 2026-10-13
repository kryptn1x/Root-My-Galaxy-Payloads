# KernelSU-Next late-load builds (A25)

This directory contains the KernelSU-Next port of the Samsung late-load chain.
The kernel module is built from KernelSU-Next `v3.4.1`
(`8f902aeb`, version code `33333`, tag `v3.4.1`) plus the repository's Samsung
KDP/RKP/DEFEX patch. It is not interchangeable between KMIs.

## Versioned artifacts

| File | Target | KMI | Purpose |
| --- | --- | --- | --- |
| `android12-5.10_kernelsu-A256EXXSADZA2-ksunext.ko` | `SM-A256E`, `A256EXXSADZA2` | `android12-5.10` | Exact A25 module with target `vermagic`, Samsung KDP/RKP/DEFEX no-patch-text build, audited for manual relocation |
| `ksud-A256EXXSADZA2-ksunext` | Same exact A25 build | `android12-5.10` | Late-load binary embedding the exact A25 module |

`Root My Galaxy` downloads the `ksud-A256EXXSADZA2-ksunext` file because
`ksud late-load` loads its embedded `android12-5.10_kernelsu.ko` asset.

## Patch contents

[`patches/KernelSU-Next-v3.4.1-samsung-kdp-rkp-defex.patch`](patches/KernelSU-Next-v3.4.1-samsung-kdp-rkp-defex.patch)
contains the complete source delta from the tagged v3.4.1 tree:

- resolve Samsung KDP credential helpers and release protected credentials with
  `kdp_usecount_dec_and_test()` plus `__put_cred()`;
- install KDP credentials through `prepare_ro_creds()` on a root workqueue and
  update the target task with the firmware-native `kdp_assign_pgd()` path;
- synchronize the DEFEX task credential record after a successful transition;
- limit the DEFEX allow path to the current UID-0 task already in `u:r:ksu:s0`;
- record a syscall-table hook only if the RKP-protected write succeeds;
- when the dispatcher is unavailable, preserve Manager FD delivery with a
  `__arm64_sys_setresuid` kretprobe and provide sucompat through address-based
  syscall kprobes without modifying the syscall table;
- mark nested sucompat calls so a handler invoking the original syscall cannot
  recursively enter the same kprobe;
- stage `ksud` at `/data/local/tmp/.ksud-stage`, rename it onto the same
  `/data` filesystem before loading the module, then finish labels/assets after
  the module is active;
- default the LKM version fallback to `33333` (`v3.4.1`) and the version-tag
  fallback to `v3.4.1` so out-of-tree builds report the same version as the
  manager.

## Build

The module is built with the `ghcr.io/ylarod/ddk-min:android12-5.10` DDK image
against its `android12-5.10` KMI tree. Two properties of the A25 target are
baked in before building:

1. The DDK default `UTS_RELEASE` is replaced with the exact target release:

   ```text
   5.10.237-android12-9-31999025-abA256EXXSADZA2
   ```

2. The stock THIN-LTO layout panics on this Samsung 5.10 kernel, so the build
   uses FULL LTO with classic CFI (the same layout as the previous KernelSU
   v3.2.5 `A256EXXSADZA2-kdp` module): the module reports a single unified
   `.text`, a zero-length `__versions` section, and `__cfi_slowpath` +
   `__cfi_check` for the device's KDP/RKP CFI expectations.

Build the module with the Samsung config set:

```sh
make -C $KDIR M="$(pwd)" src="$(pwd)" \
  CONFIG_KSU=m \
  CONFIG_KSU_SAMSUNG_KDP=y \
  CONFIG_KSU_SAMSUNG_RKP=y \
  CONFIG_KSU_SAMSUNG_DEFEX=y \
  CONFIG_KSU_SAMSUNG_NO_PATCH_TEXT=y \
  KBUILD_MODPOST_WARN=1 \
  modules -j$(nproc)
```

Validate against the recovered target ELF and strip debug sections:

```sh
python3 kernelsu/tools/audit_module_against_target.py \
  kernelsu.ko /path/to/A256EXXSADZA2/vmlinux.elf \
  /path/to/A256EXXSADZA2/Module.symvers --manual-relocation
llvm-strip -d kernelsu.ko
```

Expected result for this target:

```text
undefined symbols: 203
module version entries: 0
missing from target symbol table: 0
target CRC mismatches: 0
```

Then copy the stripped module to `userspace/ksud/bin/aarch64/android12-5.10_kernelsu.ko`
and rebuild `ksud` for `aarch64-linux-android` (see the top-level
`build.sh`/`justfile`); the late-load binary and its embedded module must always
be published together.

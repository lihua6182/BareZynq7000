# Vendored CMSIS-Core(A), ARM.CMSIS 6.3.0 (tag)

Subset of ARM-software/CMSIS_6, tag `6.3.0` (see LICENSE in this
directory, Apache-2.0). Only the files needed for Cortex-A9 bare-metal
support are kept:

    core_ca.h                     Cortex-A core support: GIC distributor/CPU
                                  interface register sets and GIC_xxx API,
                                  L2C-310 (PL310) registers and L2C_xxx API,
                                  L1C_xxx cache maintenance, MMU_xxx helpers,
                                  SCTLR/ACTLR named bit fields
    cmsis_compiler.h              compiler dispatch (-> cmsis_gcc.h for us)
    cmsis_gcc.h                   GCC intrinsics; pulls in a-profile part
    cmsis_armclang.h, cmsis_clang.h, cmsis_iccarm.h
                                  same for the other compilers (unmodified)
    cmsis_version.h               version macros
    a-profile/cmsis_gcc_a.h       GCC A-profile intrinsics (__get_CPSR, SP,
                                  FPEXC, CP15 accessor macros)
    a-profile/cmsis_cp15.h        CP15 register accessors (__get/__set_SCTLR,
                                  TTBR0, DACR, ACTLR, VBAR, CNTP timers)
    a-profile/cmsis_{armclang,clang,iccarm}_a.h
                                  same for the other compilers (unmodified)

The device configuration (core/CPU defines, GIC/L2C base addresses and the
IRQn_Type interrupt enum) lives in source/soc/zynq_cmsis.h, which is the
file user code includes (indirectly, via zynq_common.h).

Upstream: https://github.com/ARM-software/CMSIS_6 (tag 6.3.0)
Files are unmodified; do not edit them locally.

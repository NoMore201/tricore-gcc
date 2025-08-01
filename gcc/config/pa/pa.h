/* Definitions of target machine for GNU compiler, for the HP Spectrum.
   Copyright (C) 1992-2023 Free Software Foundation, Inc.
   Contributed by Michael Tiemann (tiemann@cygnus.com) of Cygnus Support
   and Tim Moore (moore@defmacro.cs.utah.edu) of the Center for
   Software Science at the University of Utah.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

/* For long call handling.  */
extern unsigned long total_code_bytes;

#define pa_cpu_attr ((enum attr_cpu)pa_cpu)

#define TARGET_PA_10 (!TARGET_PA_11 && !TARGET_PA_20)

/* Generate code for the HPPA 2.0 architecture in 64bit mode.  */
#ifndef TARGET_64BIT
#define TARGET_64BIT 0
#endif

/* Generate code for ELF32 ABI.  */
#ifndef TARGET_ELF32
#define TARGET_ELF32 0
#endif

/* Generate code for SOM 32bit ABI.  */
#ifndef TARGET_SOM
#define TARGET_SOM 0
#endif

/* HP-UX UNIX features.  */
#ifndef TARGET_HPUX
#define TARGET_HPUX 0
#endif

/* HP-UX 10.10 UNIX 95 features.  */
#ifndef TARGET_HPUX_10_10
#define TARGET_HPUX_10_10 0
#endif

/* HP-UX 11.* features (11.00, 11.11, 11.23, etc.)  */
#ifndef TARGET_HPUX_11
#define TARGET_HPUX_11 0
#endif

/* HP-UX 11i multibyte and UNIX 98 extensions.  */
#ifndef TARGET_HPUX_11_11
#define TARGET_HPUX_11_11 0
#endif

/* HP-UX 11i multibyte and UNIX 2003 extensions.  */
#ifndef TARGET_HPUX_11_31
#define TARGET_HPUX_11_31 0
#endif

/* HP-UX long double library.  */
#ifndef HPUX_LONG_DOUBLE_LIBRARY
#define HPUX_LONG_DOUBLE_LIBRARY 0
#endif

/* Sync libcall support.  */
#define TARGET_SYNC_LIBCALLS (flag_sync_libcalls)

/* The maximum size of the sync library functions supported.  DImode
   is supported on 32-bit targets using floating point loads and stores.  */
#define MAX_SYNC_LIBFUNC_SIZE 8

/* The following three defines are potential target switches.  The current
   defines are optimal given the current capabilities of GAS and GNU ld.  */

/* Define to a C expression evaluating to true to use long absolute calls.
   Currently, only the HP assembler and SOM linker support long absolute
   calls.  They are used only in non-pic code.  */
#define TARGET_LONG_ABS_CALL (TARGET_SOM && !TARGET_GAS)

/* Define to a C expression evaluating to true to use long PIC symbol
   difference calls.  Long PIC symbol difference calls are only used with
   the HP assembler and linker.  The HP assembler detects this instruction
   sequence and treats it as long pc-relative call.  Currently, GAS only
   allows a difference of two symbols in the same subspace, and it doesn't
   detect the sequence as a pc-relative call.  */
#define TARGET_LONG_PIC_SDIFF_CALL (!TARGET_GAS && TARGET_HPUX)

/* Define to a C expression evaluating to true to use SOM secondary
   definition symbols for weak support.  Linker support for secondary
   definition symbols is buggy prior to HP-UX 11.X.  */
#define TARGET_SOM_SDEF 0

/* Define to a C expression evaluating to true to save the entry value
   of SP in the current frame marker.  This is normally unnecessary.
   However, the HP-UX unwind library looks at the SAVE_SP callinfo flag.
   HP compilers don't use this flag but it is supported by the assembler.
   We set this flag to indicate that register %r3 has been saved at the
   start of the frame.  Thus, when the HP unwind library is used, we
   need to generate additional code to save SP into the frame marker.  */
#define TARGET_HPUX_UNWIND_LIBRARY 0

#ifndef TARGET_DEFAULT
#define TARGET_DEFAULT MASK_GAS
#endif

#ifndef TARGET_CPU_DEFAULT
#define TARGET_CPU_DEFAULT 0
#endif

#ifndef TARGET_SCHED_DEFAULT
#define TARGET_SCHED_DEFAULT PROCESSOR_8000
#endif

/* Support for a compile-time default CPU, et cetera.  The rules are:
   --with-schedule is ignored if -mschedule is specified.
   --with-arch is ignored if -march is specified.  */
#define OPTION_DEFAULT_SPECS \
  {"arch", "%{!march=*:-march=%(VALUE)}" }, \
  {"schedule", "%{!mschedule=*:-mschedule=%(VALUE)}" }

/* Specify the dialect of assembler to use.  New mnemonics is dialect one
   and the old mnemonics are dialect zero.  */
#define ASSEMBLER_DIALECT (TARGET_PA_20 ? 1 : 0)

/* We do not have to be compatible with dbx, so we enable gdb extensions
   by default.  */
#define DEFAULT_GDB_EXTENSIONS 1

/* Select dwarf2 as the preferred debug format.  */
#define PREFERRED_DEBUGGING_TYPE DWARF2_DEBUG

/* GDB always assumes the current function's frame begins at the value
   of the stack pointer upon entry to the current function.  Accessing
   local variables and parameters passed on the stack is done using the
   base of the frame + an offset provided by GCC.

   For functions which have frame pointers this method works fine;
   the (frame pointer) == (stack pointer at function entry) and GCC provides
   an offset relative to the frame pointer.

   This loses for functions without a frame pointer; GCC provides an offset
   which is relative to the stack pointer after adjusting for the function's
   frame size.  GDB would prefer the offset to be relative to the value of
   the stack pointer at the function's entry.  Yuk!  */
#define DEBUGGER_AUTO_OFFSET(X) \
  ((GET_CODE (X) == PLUS ? INTVAL (XEXP (X, 1)) : 0) \
    + (frame_pointer_needed ? 0 : pa_compute_frame_size (get_frame_size (), 0)))

#define DEBUGGER_ARG_OFFSET(OFFSET, X) \
  ((GET_CODE (X) == PLUS ? OFFSET : 0) \
    + (frame_pointer_needed ? 0 : pa_compute_frame_size (get_frame_size (), 0)))

#define TARGET_CPU_CPP_BUILTINS()				\
do {								\
     builtin_assert("cpu=hppa");				\
     builtin_assert("machine=hppa");				\
     builtin_define("__hppa");					\
     builtin_define("__hppa__");				\
     builtin_define("__BIG_ENDIAN__");				\
     if (TARGET_PA_20)						\
       builtin_define("_PA_RISC2_0");				\
     else if (TARGET_PA_11)					\
       builtin_define("_PA_RISC1_1");				\
     else							\
       builtin_define("_PA_RISC1_0");				\
     if (HPUX_LONG_DOUBLE_LIBRARY)				\
       builtin_define("__SIZEOF_FLOAT128__=16");		\
     if (TARGET_SOFT_FLOAT)					\
       builtin_define("__SOFTFP__");				\
} while (0)

/* An old set of OS defines for various BSD-like systems.  */
#define TARGET_OS_CPP_BUILTINS()				\
  do								\
    {								\
	builtin_define_std ("REVARGV");				\
	builtin_define_std ("hp800");				\
	builtin_define_std ("hp9000");				\
	builtin_define_std ("hp9k8");				\
	if (!c_dialect_cxx () && !flag_iso)			\
	  builtin_define ("hppa");				\
	builtin_define_std ("spectrum");			\
	builtin_define_std ("unix");				\
	builtin_assert ("system=bsd");				\
	builtin_assert ("system=unix");				\
    }								\
  while (0)

#define CC1_SPEC "%{pg:} %{p:}"

#define LINK_SPEC "%{mlinker-opt:-O} %{!shared:-u main} %{shared:-b}"

/* We don't want -lg.  */
#ifndef LIB_SPEC
#define LIB_SPEC "%{!p:%{!pg:-lc}}%{p:-lc_p}%{pg:-lc_p}"
#endif

/* Make gcc agree with <machine/ansi.h> */

#define SIZE_TYPE "unsigned int"
#define PTRDIFF_TYPE "int"
#define WCHAR_TYPE "unsigned int"
#define WCHAR_TYPE_SIZE 32

/* target machine storage layout */
typedef struct GTY(()) machine_function
{
  /* Flag indicating that a .NSUBSPA directive has been output for
     this function.  */
  int in_nsubspa;
} machine_function;

/* Define this macro if it is advisable to hold scalars in registers
   in a wider mode than that declared by the program.  In such cases, 
   the value is constrained to be within the bounds of the declared
   type, but kept valid in the wider mode.  The signedness of the
   extension may differ from that of the type.  */

#define PROMOTE_MODE(MODE,UNSIGNEDP,TYPE)  \
  if (GET_MODE_CLASS (MODE) == MODE_INT	\
      && GET_MODE_SIZE (MODE) < UNITS_PER_WORD)  	\
    (MODE) = word_mode;

/* Define this if most significant bit is lowest numbered
   in instructions that operate on numbered bit-fields.  */
#define BITS_BIG_ENDIAN 1

/* Define this if most significant byte of a word is the lowest numbered.  */
/* That is true on the HP-PA.  */
#define BYTES_BIG_ENDIAN 1

/* Define this if most significant word of a multiword number is lowest
   numbered.  */
#define WORDS_BIG_ENDIAN 1

#define MAX_BITS_PER_WORD 64

/* Width of a word, in units (bytes).  */
#define UNITS_PER_WORD (TARGET_64BIT ? 8 : 4)

/* Minimum number of units in a word.  If this is undefined, the default
   is UNITS_PER_WORD.  Otherwise, it is the constant value that is the
   smallest value that UNITS_PER_WORD can have at run-time.

   This needs to be 8 when TARGET_64BIT is true to allow building various
   TImode routines in libgcc.  However, we also need the DImode DIVMOD
   routines because they are not currently implemented in pa.md.
   
   The HP runtime specification doesn't provide the alignment requirements
   and calling conventions for TImode variables.  */
#ifdef IN_LIBGCC2
#define MIN_UNITS_PER_WORD      UNITS_PER_WORD
#else
#define MIN_UNITS_PER_WORD      4
#endif

/* The widest floating point format supported by the hardware.  Note that
   setting this influences some Ada floating point type sizes, currently
   required for GNAT to operate properly.  */
#define WIDEST_HARDWARE_FP_SIZE 64

/* Allocation boundary (in *bits*) for storing arguments in argument list.  */
#define PARM_BOUNDARY BITS_PER_WORD

/* Largest alignment required for any stack parameter, in bits.
   Don't define this if it is equal to PARM_BOUNDARY */
#define MAX_PARM_BOUNDARY BIGGEST_ALIGNMENT

/* Boundary (in *bits*) on which stack pointer is always aligned;
   certain optimizations in combine depend on this.

   The HP-UX runtime documents mandate 64-byte and 16-byte alignment for
   the stack on the 32 and 64-bit ports, respectively.  However, we
   are only guaranteed that the stack is aligned to BIGGEST_ALIGNMENT
   in main.  Thus, we treat the former as the preferred alignment.  */
#define STACK_BOUNDARY BIGGEST_ALIGNMENT
#define PREFERRED_STACK_BOUNDARY (TARGET_64BIT ? 128 : 512)

/* Allocation boundary (in *bits*) for the code of a function.  */
#define FUNCTION_BOUNDARY BITS_PER_WORD

/* Alignment of field after `int : 0' in a structure.  */
#define EMPTY_FIELD_BOUNDARY 32

/* Every structure's size must be a multiple of this.  */
#define STRUCTURE_SIZE_BOUNDARY 8

/* A bit-field declared as `int' forces `int' alignment for the struct.  */
#define PCC_BITFIELD_TYPE_MATTERS 1

/* No data type wants to be aligned rounder than this.  The long double
   type has 16-byte alignment on the 64-bit target even though it was never
   implemented in hardware.  The software implementation only needs 8-byte
   alignment.  This matches the biggest alignment of the HP compilers.  */
#define BIGGEST_ALIGNMENT (2 * BITS_PER_WORD)

/* Alignment, in bits, a C conformant malloc implementation has to provide.
   The HP-UX malloc implementation provides a default alignment of 8 bytes.
   It should be 16 bytes on the 64-bit target since long double has 16-byte
   alignment.  It can be increased with mallopt but it's non critical since
   long double was never implemented in hardware.  The glibc implementation
   currently provides 8-byte alignment.  It should be 16 bytes since various
   POSIX types such as pthread_mutex_t require 16-byte alignment.  Again,
   this is non critical since 16-byte alignment is no longer needed for
   atomic operations.  */
#define MALLOC_ABI_ALIGNMENT (TARGET_64BIT ? 128 : 64)

/* Make arrays of chars word-aligned for the same reasons.  */
#define DATA_ALIGNMENT(TYPE, ALIGN)		\
  (TREE_CODE (TYPE) == ARRAY_TYPE		\
   && TYPE_MODE (TREE_TYPE (TYPE)) == QImode	\
   && (ALIGN) < BITS_PER_WORD ? BITS_PER_WORD : (ALIGN))

/* Set this nonzero if move instructions will actually fail to work
   when given unaligned data.  */
#define STRICT_ALIGNMENT 1

/* Specify the registers used for certain standard purposes.
   The values of these macros are register numbers.  */

/* The HP-PA pc isn't overloaded on a register that the compiler knows about.  */
/* #define PC_REGNUM  */

/* Register to use for pushing function arguments.  */
#define STACK_POINTER_REGNUM 30

/* Fixed register for local variable access.  Always eliminated.  */
#define FRAME_POINTER_REGNUM (TARGET_64BIT ? 61 : 89)

/* Base register for access to local variables of the function.  */
#define HARD_FRAME_POINTER_REGNUM 3

/* Don't allow hard registers to be renamed into r2 unless r2
   is already live or already being saved (due to eh).  */

#define HARD_REGNO_RENAME_OK(OLD_REG, NEW_REG) \
  ((NEW_REG) != 2 || df_regs_ever_live_p (2) || crtl->calls_eh_return)

/* Base register for access to arguments of the function.  */
#define ARG_POINTER_REGNUM (TARGET_64BIT ? 29 : 3)

/* Register in which static-chain is passed to a function.  */
#define STATIC_CHAIN_REGNUM (TARGET_64BIT ? 31 : 29)

/* Register used to address the offset table for position-independent
   data references.  */
#define PIC_OFFSET_TABLE_REGNUM \
  (flag_pic ? (TARGET_64BIT ? 27 : 19) : INVALID_REGNUM)

#define PIC_OFFSET_TABLE_REG_CALL_CLOBBERED 1

/* Function to return the rtx used to save the pic offset table register
   across function calls.  */
extern rtx hppa_pic_save_rtx (void);

#define DEFAULT_PCC_STRUCT_RETURN 0

/* Register in which address to store a structure value
   is passed to a function.  */
#define PA_STRUCT_VALUE_REGNUM 28

/* Definitions for register eliminations.

   We have two registers that can be eliminated.  First, the frame pointer
   register can often be eliminated in favor of the stack pointer register.
   Secondly, the argument pointer register can always be eliminated in the
   32-bit runtimes.  */

/* This is an array of structures.  Each structure initializes one pair
   of eliminable registers.  The "from" register number is given first,
   followed by "to".  Eliminations of the same "from" register are listed
   in order of preference.

   The argument pointer cannot be eliminated in the 64-bit runtime.  It
   is the same register as the hard frame pointer in the 32-bit runtime.
   So, it does not need to be listed.  */
#define ELIMINABLE_REGS                                 \
{{ HARD_FRAME_POINTER_REGNUM, STACK_POINTER_REGNUM},    \
 { FRAME_POINTER_REGNUM, STACK_POINTER_REGNUM},         \
 { FRAME_POINTER_REGNUM, HARD_FRAME_POINTER_REGNUM} }

/* Define the offset between two registers, one to be eliminated,
   and the other its replacement, at the start of a routine.  */
#define INITIAL_ELIMINATION_OFFSET(FROM, TO, OFFSET) \
  ((OFFSET) = pa_initial_elimination_offset(FROM, TO))

/* Describe how we implement __builtin_eh_return.  */
#define EH_RETURN_DATA_REGNO(N)	\
  ((N) < 3 ? (N) + 20 : (N) == 3 ? 31 : INVALID_REGNUM)
#define EH_RETURN_STACKADJ_RTX	gen_rtx_REG (Pmode, 29)
#define EH_RETURN_HANDLER_RTX pa_eh_return_handler_rtx ()

/* Offset from the frame pointer register value to the top of stack.  */
#define FRAME_POINTER_CFA_OFFSET(FNDECL) 0

/* The maximum number of hard registers that can be saved in the call
   frame.  The soft frame pointer is not included.  */
#define DWARF_FRAME_REGISTERS (FIRST_PSEUDO_REGISTER - 1)

/* A C expression whose value is RTL representing the location of the
   incoming return address at the beginning of any function, before the
   prologue.  You only need to define this macro if you want to support
   call frame debugging information like that provided by DWARF 2.  */
#define INCOMING_RETURN_ADDR_RTX (gen_rtx_REG (word_mode, 2))
#define DWARF_FRAME_RETURN_COLUMN (DWARF_FRAME_REGNUM (2))

/* A C expression whose value is an integer giving a DWARF 2 column
   number that may be used as an alternate return column.  This should
   be defined only if DWARF_FRAME_RETURN_COLUMN is set to a general
   register, but an alternate column needs to be used for signal frames.

   Column 0 is not used but unfortunately its register size is set to
   4 bytes (sizeof CCmode) so it can't be used on 64-bit targets.  */
#define DWARF_ALT_FRAME_RETURN_COLUMN (FIRST_PSEUDO_REGISTER - 1)

/* This macro chooses the encoding of pointers embedded in the exception
   handling sections.  If at all possible, this should be defined such
   that the exception handling section will not require dynamic relocations,
   and so may be read-only.

   Because the HP assembler auto aligns, it is necessary to use
   DW_EH_PE_aligned.  It's not possible to make the data read-only
   on the HP-UX SOM port since the linker requires fixups for label
   differences in different sections to be word aligned.  However,
   the SOM linker can do unaligned fixups for absolute pointers.
   We also need aligned pointers for global and function pointers.

   Although the HP-UX 64-bit ELF linker can handle unaligned pc-relative
   fixups, the runtime doesn't have a consistent relationship between
   text and data for dynamically loaded objects.  Thus, it's not possible
   to use pc-relative encoding for pointers on this target.  It may be
   possible to use segment relative encodings but GAS doesn't currently
   have a mechanism to generate these encodings.  For other targets, we
   use pc-relative encoding for pointers.  If the pointer might require
   dynamic relocation, we make it indirect.  */
#define ASM_PREFERRED_EH_DATA_FORMAT(CODE,GLOBAL)			\
  (TARGET_GAS && !TARGET_HPUX						\
   ? (DW_EH_PE_pcrel							\
      | ((GLOBAL) || (CODE) == 2 ? DW_EH_PE_indirect : 0)		\
      | (TARGET_64BIT ? DW_EH_PE_sdata8 : DW_EH_PE_sdata4))		\
   : (!TARGET_GAS || (GLOBAL) || (CODE) == 2				\
      ? DW_EH_PE_aligned : DW_EH_PE_absptr))

/* Handle special EH pointer encodings.  Absolute, pc-relative, and
   indirect are handled automatically.  We output pc-relative, and
   indirect pc-relative ourself since we need some special magic to
   generate pc-relative relocations, and to handle indirect function
   pointers.  */
#define ASM_MAYBE_OUTPUT_ENCODED_ADDR_RTX(FILE, ENCODING, SIZE, ADDR, DONE) \
  do {									\
    if (((ENCODING) & 0x70) == DW_EH_PE_pcrel)				\
      {									\
	fputs (integer_asm_op (SIZE, FALSE), FILE);			\
	if ((ENCODING) & DW_EH_PE_indirect)				\
	  output_addr_const (FILE, pa_get_deferred_plabel (ADDR));	\
	else								\
	  assemble_name (FILE, XSTR ((ADDR), 0));			\
	fputs ("+8-$PIC_pcrel$0", FILE);				\
	goto DONE;							\
      }									\
    } while (0)


/* The class value for index registers, and the one for base regs.  */
#define INDEX_REG_CLASS GENERAL_REGS
#define BASE_REG_CLASS GENERAL_REGS

/* True if register is a general register.  */
#define GENERAL_REGNO_P(N) ((N) >= 1 && (N) <= 31)

#define FP_REG_CLASS_P(CLASS) \
  ((CLASS) == FP_REGS || (CLASS) == FPUPPER_REGS)

/* True if register is floating-point.  */
#define FP_REGNO_P(N) ((N) >= FP_REG_FIRST && (N) <= FP_REG_LAST)

#define MAYBE_FP_REG_CLASS_P(CLASS) \
  reg_classes_intersect_p ((CLASS), FP_REGS)


/* Stack layout; function entry, exit and calling.  */

/* Define this if pushing a word on the stack
   makes the stack pointer a smaller address.  */
/* #define STACK_GROWS_DOWNWARD */

/* Believe it or not.  */
#define ARGS_GROW_DOWNWARD 1

/* Define this to nonzero if the nominal address of the stack frame
   is at the high-address end of the local variables;
   that is, each additional local variable allocated
   goes at a more negative offset in the frame.  */
#define FRAME_GROWS_DOWNWARD 0

/* Define STACK_ALIGNMENT_NEEDED to zero to disable final alignment
   of the stack.  The default is to align it to STACK_BOUNDARY.  */
#define STACK_ALIGNMENT_NEEDED 0

/* If we generate an insn to push BYTES bytes,
   this says how many the stack pointer really advances by.
   On the HP-PA, don't define this because there are no push insns.  */
/*  #define PUSH_ROUNDING(BYTES) */

/* Offset of first parameter from the argument pointer register value.
   This value will be negated because the arguments grow down.
   Also note that on STACK_GROWS_UPWARD machines (such as this one)
   this is the distance from the frame pointer to the end of the first
   argument, not it's beginning.  To get the real offset of the first
   argument, the size of the argument must be added.  */

#define FIRST_PARM_OFFSET(FNDECL) (TARGET_64BIT ? -64 : -32)

/* When a parameter is passed in a register, stack space is still
   allocated for it.  */
#define REG_PARM_STACK_SPACE(DECL) (TARGET_64BIT ? 64 : 16)

/* Define this if the above stack space is to be considered part of the
   space allocated by the caller.  */
#define OUTGOING_REG_PARM_STACK_SPACE(FNTYPE) 1

/* Keep the stack pointer constant throughout the function.
   This is both an optimization and a necessity: longjmp
   doesn't behave itself when the stack pointer moves within
   the function!  */
#define ACCUMULATE_OUTGOING_ARGS 1

/* The weird HPPA calling conventions require a minimum of 48 bytes on
   the stack: 16 bytes for register saves, and 32 bytes for magic.
   This is the difference between the logical top of stack and the
   actual sp.

   On the 64-bit port, the HP C compiler allocates a 48-byte frame
   marker, although the runtime documentation only describes a 16
   byte marker.  For compatibility, we allocate 48 bytes.  */
#define STACK_POINTER_OFFSET \
  (TARGET_64BIT ? -(crtl->outgoing_args_size + 48) : poly_int64 (-32))

#define STACK_DYNAMIC_OFFSET(FNDECL)	\
  (TARGET_64BIT				\
   ? (STACK_POINTER_OFFSET)		\
   : ((STACK_POINTER_OFFSET) - crtl->outgoing_args_size))


/* Define a data type for recording info about an argument list
   during the scan of that argument list.  This data type should
   hold all necessary information about the function itself
   and about the args processed so far, enough to enable macros
   such as FUNCTION_ARG to determine where the next arg should go.

   On the HP-PA, the WORDS field holds the number of words
   of arguments scanned so far (including the invisible argument,
   if any, which holds the structure-value-address).  Thus, 4 or
   more means all following args should go on the stack.
   
   The INCOMING field tracks whether this is an "incoming" or
   "outgoing" argument.
   
   The INDIRECT field indicates whether this is an indirect
   call or not.
   
   The NARGS_PROTOTYPE field indicates that an argument does not
   have a prototype when it less than or equal to 0.  */

struct hppa_args {int words, nargs_prototype, incoming, indirect; };

#define CUMULATIVE_ARGS struct hppa_args

/* Initialize a variable CUM of type CUMULATIVE_ARGS
   for a call to a function whose data type is FNTYPE.
   For a library call, FNTYPE is 0.  */

#define INIT_CUMULATIVE_ARGS(CUM, FNTYPE, LIBNAME, FNDECL, N_NAMED_ARGS) \
  (CUM).words = 0, 							\
  (CUM).incoming = 0,							\
  (CUM).indirect = (FNTYPE) && !(FNDECL),				\
  (CUM).nargs_prototype = (FNTYPE && prototype_p (FNTYPE)		\
			   ? (list_length (TYPE_ARG_TYPES (FNTYPE)) - 1	\
			      + (TYPE_MODE (TREE_TYPE (FNTYPE)) == BLKmode \
				 || pa_return_in_memory (TREE_TYPE (FNTYPE), 0))) \
			   : 0)



/* Similar, but when scanning the definition of a procedure.  We always
   set NARGS_PROTOTYPE large so we never return a PARALLEL.  */

#define INIT_CUMULATIVE_INCOMING_ARGS(CUM,FNTYPE,IGNORE) \
  (CUM).words = 0,				\
  (CUM).incoming = 1,				\
  (CUM).indirect = 0,				\
  (CUM).nargs_prototype = 1000

/* Determine where to put an argument to a function.
   Value is zero to push the argument on the stack,
   or a hard register in which to store the argument.

   MODE is the argument's machine mode.
   TYPE is the data type of the argument (as a tree).
    This is null for libcalls where that information may
    not be available.
   CUM is a variable of type CUMULATIVE_ARGS which gives info about
    the preceding args and about the function being called.
   NAMED is nonzero if this argument is a named parameter
    (otherwise it is an extra parameter matching an ellipsis).

   On the HP-PA the first four words of args are normally in registers
   and the rest are pushed.  But any arg that won't entirely fit in regs
   is pushed.

   Arguments passed in registers are either 1 or 2 words long.

   The caller must make a distinction between calls to explicitly named
   functions and calls through pointers to functions -- the conventions
   are different!  Calls through pointers to functions only use general
   registers for the first four argument words.

   Of course all this is different for the portable runtime model
   HP wants everyone to use for ELF.  Ugh.  Here's a quick description
   of how it's supposed to work.

   1) callee side remains unchanged.  It expects integer args to be
   in the integer registers, float args in the float registers and
   unnamed args in integer registers.

   2) caller side now depends on if the function being called has
   a prototype in scope (rather than if it's being called indirectly).

      2a) If there is a prototype in scope, then arguments are passed
      according to their type (ints in integer registers, floats in float
      registers, unnamed args in integer registers.

      2b) If there is no prototype in scope, then floating point arguments
      are passed in both integer and float registers.  egad.

  FYI: The portable parameter passing conventions are almost exactly like
  the standard parameter passing conventions on the RS6000.  That's why
  you'll see lots of similar code in rs6000.h.  */

/* Specify padding for the last element of a block move between registers
   and memory.

   The 64-bit runtime specifies that objects need to be left justified
   (i.e., the normal justification for a big endian target).  The 32-bit
   runtime specifies right justification for objects smaller than 64 bits.
   We use a DImode register in the parallel for 5 to 7 byte structures
   so that there is only one element.  This allows the object to be
   correctly padded.  */
#define BLOCK_REG_PADDING(MODE, TYPE, FIRST) \
  targetm.calls.function_arg_padding ((MODE), (TYPE))


/* On HPPA, we emit profiling code as rtl via PROFILE_HOOK rather than
   as assembly via FUNCTION_PROFILER.  Just output a local label.
   We can't use the function label because the GAS SOM target can't
   handle the difference of a global symbol and a local symbol.  */

#ifndef FUNC_BEGIN_PROLOG_LABEL
#define FUNC_BEGIN_PROLOG_LABEL        "LFBP"
#endif

#define FUNCTION_PROFILER(FILE, LABEL) \
  (*targetm.asm_out.internal_label) (FILE, FUNC_BEGIN_PROLOG_LABEL, LABEL)

#define PROFILE_HOOK(label_no) hppa_profile_hook (label_no)

/* The profile counter if emitted must come before the prologue.  */
#define PROFILE_BEFORE_PROLOGUE 1

/* We never want final.cc to emit profile counters.  When profile
   counters are required, we have to defer emitting them to the end
   of the current file.  */
#define NO_PROFILE_COUNTERS 1

/* EXIT_IGNORE_STACK should be nonzero if, when returning from a function,
   the stack pointer does not matter.  The value is tested only in
   functions that have frame pointers.
   No definition is equivalent to always zero.  */

extern int may_call_alloca;

#define EXIT_IGNORE_STACK	\
 (maybe_ne (get_frame_size (), 0)	\
  || cfun->calls_alloca || maybe_ne (crtl->outgoing_args_size, 0))

/* Length in units of the trampoline for entering a nested function.  */

#define TRAMPOLINE_SIZE (TARGET_64BIT ? 72 : 64)

/* Alignment required by the trampoline.  */

#define TRAMPOLINE_ALIGNMENT BITS_PER_WORD

/* Minimum length of a cache line.  A length of 16 will work on all
   PA-RISC processors.  All PA 1.1 processors have a cache line of
   32 bytes.  Most but not all PA 2.0 processors have a cache line
   of 64 bytes.  As cache flushes are expensive and we don't support
   PA 1.0, we use a minimum length of 32.  */

#define MIN_CACHELINE_SIZE 32


/* Addressing modes, and classification of registers for them. 

   Using autoincrement addressing modes on PA8000 class machines is
   not profitable.  */

#define HAVE_POST_INCREMENT (pa_cpu < PROCESSOR_8000)
#define HAVE_POST_DECREMENT (pa_cpu < PROCESSOR_8000)

#define HAVE_PRE_DECREMENT (pa_cpu < PROCESSOR_8000)
#define HAVE_PRE_INCREMENT (pa_cpu < PROCESSOR_8000)

/* Macros to check register numbers against specific register classes.  */

/* The following macros assume that X is a hard or pseudo reg number.
   They give nonzero only if X is a hard reg of the suitable class
   or a pseudo reg currently allocated to a suitable hard reg.
   Since they use reg_renumber, they are safe only once reg_renumber
   has been allocated, which happens in reginfo.cc during register
   allocation.  */

#define REGNO_OK_FOR_INDEX_P(X) \
  ((X) && ((X) < 32							\
   || ((X) == FRAME_POINTER_REGNUM)					\
   || ((X) >= FIRST_PSEUDO_REGISTER					\
       && reg_renumber							\
       && (unsigned) reg_renumber[X] < 32)))
#define REGNO_OK_FOR_BASE_P(X) \
  ((X) && ((X) < 32							\
   || ((X) == FRAME_POINTER_REGNUM)					\
   || ((X) >= FIRST_PSEUDO_REGISTER					\
       && reg_renumber							\
       && (unsigned) reg_renumber[X] < 32)))
#define REGNO_OK_FOR_FP_P(X) \
  (FP_REGNO_P (X)							\
   || (X >= FIRST_PSEUDO_REGISTER					\
       && reg_renumber							\
       && FP_REGNO_P (reg_renumber[X])))

/* Now macros that check whether X is a register and also,
   strictly, whether it is in a specified class.

   These macros are specific to the HP-PA, and may be used only
   in code for printing assembler insns and in conditions for
   define_optimization.  */

/* 1 if X is an fp register.  */

#define FP_REG_P(X) (REG_P (X) && REGNO_OK_FOR_FP_P (REGNO (X)))

/* Maximum number of registers that can appear in a valid memory address.  */

#define MAX_REGS_PER_ADDRESS 2

/* TLS symbolic reference.  */
#define PA_SYMBOL_REF_TLS_P(X) \
  (GET_CODE (X) == SYMBOL_REF && SYMBOL_REF_TLS_MODEL (X) != 0)

/* Recognize any constant value that is a valid address except
   for symbolic addresses.  We get better CSE by rejecting them
   here and allowing hppa_legitimize_address to break them up.  We
   use most of the constants accepted by CONSTANT_P, except CONST_DOUBLE.  */

#define CONSTANT_ADDRESS_P(X) \
  ((GET_CODE (X) == LABEL_REF 						\
   || (GET_CODE (X) == SYMBOL_REF && !SYMBOL_REF_TLS_MODEL (X))		\
   || GET_CODE (X) == CONST_INT						\
   || (GET_CODE (X) == CONST && !tls_referenced_p (X))			\
   || GET_CODE (X) == HIGH) 						\
   && (reload_in_progress || reload_completed				\
       || ! pa_symbolic_expression_p (X)))

/* A C expression that is nonzero if we are using the new HP assembler.  */

#ifndef NEW_HP_ASSEMBLER
#define NEW_HP_ASSEMBLER 0
#endif

/* The macros below define the immediate range for CONST_INTS on
   the 64-bit port.  Constants in this range can be loaded in three
   instructions using a ldil/ldo/depdi sequence.  Constants outside
   this range are forced to the constant pool prior to reload.  */

#define MAX_LEGIT_64BIT_CONST_INT ((HOST_WIDE_INT) 32 << 31)
#define MIN_LEGIT_64BIT_CONST_INT \
  ((HOST_WIDE_INT)((unsigned HOST_WIDE_INT) -32 << 31))
#define LEGITIMATE_64BIT_CONST_INT_P(X) \
  ((X) >= MIN_LEGIT_64BIT_CONST_INT && (X) < MAX_LEGIT_64BIT_CONST_INT)

/* Target flags set on a symbol_ref.  */

/* Set by ASM_OUTPUT_SYMBOL_REF when a symbol_ref is output.  */
#define SYMBOL_FLAG_REFERENCED (1 << SYMBOL_FLAG_MACH_DEP_SHIFT)
#define SYMBOL_REF_REFERENCED_P(RTX) \
  ((SYMBOL_REF_FLAGS (RTX) & SYMBOL_FLAG_REFERENCED) != 0)

/* Defines for constraints.md.  */

/* Return 1 iff OP is a scaled or unscaled index address.  */
#define IS_INDEX_ADDR_P(OP) \
  (GET_CODE (OP) == PLUS				\
   && GET_MODE (OP) == Pmode				\
   && (GET_CODE (XEXP (OP, 0)) == MULT			\
       || GET_CODE (XEXP (OP, 1)) == MULT		\
       || (REG_P (XEXP (OP, 0))				\
	   && REG_P (XEXP (OP, 1)))))

/* Return 1 iff OP is a LO_SUM DLT address.  */
#define IS_LO_SUM_DLT_ADDR_P(OP) \
  (GET_CODE (OP) == LO_SUM				\
   && GET_MODE (OP) == Pmode				\
   && REG_P (XEXP (OP, 0))				\
   && REG_OK_FOR_BASE_P (XEXP (OP, 0))			\
   && GET_CODE (XEXP (OP, 1)) == UNSPEC)

/* Nonzero if 14-bit offsets can be used for all loads and stores.
   This is not possible when generating PA 1.x code as floating point
   loads and stores only support 5-bit offsets.  Note that we do not
   forbid the use of 14-bit offsets for integer modes.  Instead, we
   use secondary reloads to fix REG+D memory addresses for integer
   mode floating-point loads and stores.

   FIXME: the ELF32 linker clobbers the LSB of the FP register number
   in PA 2.0 floating-point insns with long displacements.  This is
   because R_PARISC_DPREL14WR and other relocations like it are not
   yet supported by GNU ld.  For now, we reject long displacements
   on this target.  */

#define INT14_OK_STRICT \
  (TARGET_SOFT_FLOAT                                                   \
   || (TARGET_PA_20 && !TARGET_ELF32))

/* The macros REG_OK_FOR..._P assume that the arg is a REG rtx
   and check its validity for a certain class.
   We have two alternate definitions for each of them.
   The usual definition accepts all pseudo regs; the other rejects
   them unless they have been allocated suitable hard regs.

   Most source files want to accept pseudo regs in the hope that
   they will get allocated to the class that the insn wants them to be in.
   Source files for reload pass need to be strict.
   After reload, it makes no difference, since pseudo regs have
   been eliminated by then.  */

/* Nonzero if X is a hard reg that can be used as an index
   or if it is a pseudo reg.  */
#define REG_OK_FOR_INDEX_P(X) \
  (REGNO (X) && (REGNO (X) < 32 				\
   || REGNO (X) == FRAME_POINTER_REGNUM				\
   || REGNO (X) >= FIRST_PSEUDO_REGISTER))

/* Nonzero if X is a hard reg that can be used as a base reg
   or if it is a pseudo reg.  */
#define REG_OK_FOR_BASE_P(X) \
  (REGNO (X) && (REGNO (X) < 32 				\
   || REGNO (X) == FRAME_POINTER_REGNUM				\
   || REGNO (X) >= FIRST_PSEUDO_REGISTER))

/* Nonzero if X is a hard reg that can be used as an index.  */
#define STRICT_REG_OK_FOR_INDEX_P(X) REGNO_OK_FOR_INDEX_P (REGNO (X))

/* Nonzero if X is a hard reg that can be used as a base reg.  */
#define STRICT_REG_OK_FOR_BASE_P(X) REGNO_OK_FOR_BASE_P (REGNO (X))

#define VAL_5_BITS_P(X) ((unsigned HOST_WIDE_INT)(X) + 0x10 < 0x20)
#define INT_5_BITS(X) VAL_5_BITS_P (INTVAL (X))

#define VAL_U5_BITS_P(X) ((unsigned HOST_WIDE_INT)(X) < 0x20)
#define INT_U5_BITS(X) VAL_U5_BITS_P (INTVAL (X))

#define VAL_U6_BITS_P(X) ((unsigned HOST_WIDE_INT)(X) < 0x40)
#define INT_U6_BITS(X) VAL_U6_BITS_P (INTVAL (X))

#define VAL_11_BITS_P(X) ((unsigned HOST_WIDE_INT)(X) + 0x400 < 0x800)
#define INT_11_BITS(X) VAL_11_BITS_P (INTVAL (X))

#define VAL_14_BITS_P(X) ((unsigned HOST_WIDE_INT)(X) + 0x2000 < 0x4000)
#define INT_14_BITS(X) VAL_14_BITS_P (INTVAL (X))

#if HOST_BITS_PER_WIDE_INT > 32
#define VAL_32_BITS_P(X) \
  ((unsigned HOST_WIDE_INT)(X) + ((unsigned HOST_WIDE_INT) 1 << 31)    \
   < (unsigned HOST_WIDE_INT) 2 << 31)
#else
#define VAL_32_BITS_P(X) 1
#endif
#define INT_32_BITS(X) VAL_32_BITS_P (INTVAL (X))

/* These are the modes that we allow for scaled indexing.  */
#define MODE_OK_FOR_SCALED_INDEXING_P(MODE) \
  ((TARGET_64BIT && (MODE) == DImode)					\
   || (MODE) == SImode							\
   || (MODE) == HImode							\
   || (MODE) == SFmode							\
   || (MODE) == DFmode)

/* These are the modes that we allow for unscaled indexing.  */
#define MODE_OK_FOR_UNSCALED_INDEXING_P(MODE) \
  ((TARGET_64BIT && (MODE) == DImode)					\
   || (MODE) == SImode							\
   || (MODE) == HImode							\
   || (MODE) == QImode							\
   || (MODE) == SFmode							\
   || (MODE) == DFmode)

/* Try a machine-dependent way of reloading an illegitimate address
   operand.  If we find one, push the reload and jump to WIN.  This
   macro is used in only one place: `find_reloads_address' in reload.cc.  */

#define LEGITIMIZE_RELOAD_ADDRESS(AD, MODE, OPNUM, TYPE, IND_L, WIN) 	     \
do {									     \
  rtx new_ad = pa_legitimize_reload_address (AD, MODE, OPNUM, TYPE, IND_L);  \
  if (new_ad)								     \
    {									     \
      AD = new_ad;							     \
      goto WIN;								     \
    }									     \
} while (0)


#define TARGET_ASM_SELECT_SECTION  pa_select_section

/* Return a nonzero value if DECL has a section attribute.  */
#define IN_NAMED_SECTION_P(DECL) \
  ((TREE_CODE (DECL) == FUNCTION_DECL || TREE_CODE (DECL) == VAR_DECL) \
   && DECL_SECTION_NAME (DECL) != NULL)

/* Define this macro if references to a symbol must be treated
   differently depending on something about the variable or
   function named by the symbol (such as what section it is in).

   The macro definition, if any, is executed immediately after the
   rtl for DECL or other node is created.
   The value of the rtl will be a `mem' whose address is a
   `symbol_ref'.

   The usual thing for this macro to do is to a flag in the
   `symbol_ref' (such as `SYMBOL_REF_FLAG') or to store a modified
   name string in the `symbol_ref' (if one bit is not enough
   information).

   On the HP-PA we use this to indicate if a symbol is in text or
   data space.  Also, function labels need special treatment.  */

#define TEXT_SPACE_P(DECL)\
  (TREE_CODE (DECL) == FUNCTION_DECL					\
   || (TREE_CODE (DECL) == VAR_DECL					\
       && TREE_READONLY (DECL) && ! TREE_SIDE_EFFECTS (DECL)		\
       && (! DECL_INITIAL (DECL) || ! pa_reloc_needed (DECL_INITIAL (DECL))) \
       && !flag_pic)							\
   || CONSTANT_CLASS_P (DECL))

#define FUNCTION_NAME_P(NAME)  (*(NAME) == '@')

/* Specify the machine mode that this machine uses for the index in the
   tablejump instruction.  We use a 32-bit absolute address for non-pic code,
   and a 32-bit offset for 32 and 64-bit pic code.  */
#define CASE_VECTOR_MODE SImode

/* Jump tables must be 32-bit aligned, no matter the size of the element.  */
#define ADDR_VEC_ALIGN(ADDR_VEC) 2

/* Define this as 1 if `char' should by default be signed; else as 0.  */
#define DEFAULT_SIGNED_CHAR 1

/* Max number of bytes we can move from memory to memory
   in one reasonably fast instruction.  */
#define MOVE_MAX 8

/* Higher than the default as we prefer to use simple move insns
   (better scheduling and delay slot filling) and because our
   built-in block move is really a 2X unrolled loop. 

   Believe it or not, this has to be big enough to allow for copying all
   arguments passed in registers to avoid infinite recursion during argument
   setup for a function call.  Why?  Consider how we copy the stack slots
   reserved for parameters when they may be trashed by a call.  */
#define MOVE_RATIO(speed) (TARGET_64BIT ? 8 : 4)

/* Define if operations between registers always perform the operation
   on the full register even if a narrower mode is specified.  */
#define WORD_REGISTER_OPERATIONS 1

/* Define if loading in MODE, an integral mode narrower than BITS_PER_WORD
   will either zero-extend or sign-extend.  The value of this macro should
   be the code that says which one of the two operations is implicitly
   done, UNKNOWN if none.  */
#define LOAD_EXTEND_OP(MODE) ZERO_EXTEND

/* Nonzero if access to memory by bytes is slow and undesirable.  */
#define SLOW_BYTE_ACCESS 1

/* Specify the machine mode that pointers have.
   After generation of rtl, the compiler makes no further distinction
   between pointers and any other objects of this machine mode.  */
#define Pmode word_mode

/* Given a comparison code (EQ, NE, etc.) and the first operand of a COMPARE,
   return the mode to be used for the comparison.  For floating-point, CCFPmode
   should be used.  CC_NOOVmode should be used when the first operand is a
   PLUS, MINUS, or NEG.  CCmode should be used when no special processing is
   needed.  */
#define SELECT_CC_MODE(OP,X,Y) \
  (GET_MODE_CLASS (GET_MODE (X)) == MODE_FLOAT ? CCFPmode : CCmode)    \

/* A function address in a call instruction
   is a byte address (for indexing purposes)
   so give the MEM rtx a byte's mode.  */
#define FUNCTION_MODE SImode

/* Define this if addresses of constant functions
   shouldn't be put through pseudo regs where they can be cse'd.
   Desirable on machines where ordinary constants are expensive
   but a CALL with constant address is cheap.  */
#define NO_FUNCTION_CSE 1

/* Define this to be nonzero if shift instructions ignore all but the low-order
   few bits.  */
#define SHIFT_COUNT_TRUNCATED 1

/* Adjust the cost of branches.  */
#define BRANCH_COST(speed_p, predictable_p) (pa_cpu == PROCESSOR_8000 ? 2 : 1)

/* Handling the special cases is going to get too complicated for a macro,
   just call `pa_adjust_insn_length' to do the real work.  */
#define ADJUST_INSN_LENGTH(INSN, LENGTH) \
  ((LENGTH) = pa_adjust_insn_length ((INSN), (LENGTH)))

/* Millicode insns are actually function calls with some special
   constraints on arguments and register usage.

   Millicode calls always expect their arguments in the integer argument
   registers, and always return their result in %r29 (ret1).  They
   are expected to clobber their arguments, %r1, %r29, and the return
   pointer which is %r31 on 32-bit and %r2 on 64-bit, and nothing else.

   This macro tells reorg that the references to arguments and
   millicode calls do not appear to happen until after the millicode call.
   This allows reorg to put insns which set the argument registers into the
   delay slot of the millicode call -- thus they act more like traditional
   CALL_INSNs.

   Note we cannot consider side effects of the insn to be delayed because
   the branch and link insn will clobber the return pointer.  If we happened
   to use the return pointer in the delay slot of the call, then we lose.

   get_attr_type will try to recognize the given insn, so make sure to
   filter out things it will not accept -- SEQUENCE, USE and CLOBBER insns
   in particular.  */
#define INSN_REFERENCES_ARE_DELAYED(X) (pa_insn_refs_are_delayed (X))


/* Control the assembler format that we output.  */

/* A C string constant describing how to begin a comment in the target
   assembler language.  The compiler assumes that the comment will end at
   the end of the line.  */

#define ASM_COMMENT_START ";"

/* Output to assembler file text saying following lines
   may contain character constants, extra white space, comments, etc.  */

#define ASM_APP_ON ""

/* Output to assembler file text saying following lines
   no longer contain unusual constructs.  */

#define ASM_APP_OFF ""

/* This is how to output the definition of a user-level label named NAME,
   such as the label on a static function or variable NAME.  */

#define ASM_OUTPUT_LABEL(FILE,NAME) \
  do {							\
    assemble_name ((FILE), (NAME));			\
    if (TARGET_GAS)					\
      fputs (":\n", (FILE));				\
    else						\
      fputc ('\n', (FILE));				\
  } while (0)

/* This is how to output a reference to a user-level label named NAME.
   `assemble_name' uses this.  */

#define ASM_OUTPUT_LABELREF(FILE,NAME)	\
  do {					\
    const char *xname = (NAME);		\
    if (FUNCTION_NAME_P (NAME))		\
      xname += 1;			\
    if (xname[0] == '*')		\
      xname += 1;			\
    else				\
      fputs (user_label_prefix, FILE);	\
    fputs (xname, FILE);		\
  } while (0)

/* This how we output the symbol_ref X.  */

#define ASM_OUTPUT_SYMBOL_REF(FILE,X) \
  do {                                                 \
    SYMBOL_REF_FLAGS (X) |= SYMBOL_FLAG_REFERENCED;    \
    assemble_name (FILE, XSTR (X, 0));                 \
  } while (0)

/* This is how to store into the string LABEL
   the symbol_ref name of an internal numbered label where
   PREFIX is the class of label and NUM is the number within the class.
   This is suitable for output with `assemble_name'.  */

#define ASM_GENERATE_INTERNAL_LABEL(LABEL, PREFIX, NUM)		\
  do								\
    {								\
      char *__p;						\
      (LABEL)[0] = '*';						\
      (LABEL)[1] = (PREFIX)[0];					\
      (LABEL)[2] = '$';						\
      __p = stpcpy (&(LABEL)[3], &(PREFIX)[1]);			\
      sprint_ul (__p, (unsigned long) (NUM));			\
    }								\
  while (0)


/* Output the definition of a compiler-generated label named NAME.  */

#define ASM_OUTPUT_INTERNAL_LABEL(FILE,NAME) \
  do {							\
    assemble_name_raw ((FILE), (NAME));			\
    if (TARGET_GAS)					\
      fputs (":\n", (FILE));				\
    else						\
      fputc ('\n', (FILE));				\
  } while (0)

#define TARGET_ASM_GLOBALIZE_LABEL pa_globalize_label

#define ASM_OUTPUT_ASCII(FILE, P, SIZE)  \
  pa_output_ascii ((FILE), (P), (SIZE))

/* Jump tables are always placed in the text section.  We have to do
   this for the HP-UX SOM target as we can't switch sections in the
   middle of a function.

   On ELF targets, it is possible to put them in the readonly-data section.
   This would get the table out of .text and reduce branch lengths.

   A downside is that an additional insn (addil) is needed to access
   the table when generating PIC code.  The address difference table
   also has to use 32-bit pc-relative relocations.

   The table entries need to look like "$L1+(.+8-$L0)-$PIC_pcrel$0"
   when using ELF GAS.  A simple difference can be used when using
   the HP assembler.

   The final downside is GDB complains about the nesting of the label
   for the table.  */

#define JUMP_TABLES_IN_TEXT_SECTION 1

/* This is how to output an element of a case-vector that is absolute.  */

#define ASM_OUTPUT_ADDR_VEC_ELT(FILE, VALUE)  \
  fprintf (FILE, "\t.word L$%d\n", VALUE)

/* This is how to output an element of a case-vector that is relative. 
   Since we always place jump tables in the text section, the difference
   is absolute and requires no relocation.  */

#define ASM_OUTPUT_ADDR_DIFF_ELT(FILE, BODY, VALUE, REL)  \
  fprintf (FILE, "\t.word L$%d-L$%d\n", VALUE, REL)

/* This is how to output an absolute case-vector.  */

#define ASM_OUTPUT_ADDR_VEC(LAB,BODY)	\
  pa_output_addr_vec ((LAB),(BODY))

/* This is how to output a relative case-vector.  */

#define ASM_OUTPUT_ADDR_DIFF_VEC(LAB,BODY)	\
  pa_output_addr_diff_vec ((LAB),(BODY))

/* This is how to output an assembler line that says to advance the
   location counter to a multiple of 2**LOG bytes.  */

#define ASM_OUTPUT_ALIGN(FILE,LOG)	\
    fprintf (FILE, "\t.align %d\n", (1 << (LOG)))

#define ASM_OUTPUT_SKIP(FILE,SIZE)  \
  fprintf (FILE, "\t.blockz " HOST_WIDE_INT_PRINT_UNSIGNED"\n",		\
	   (unsigned HOST_WIDE_INT)(SIZE))

/* This says how to output an assembler line to define an uninitialized
   global variable with size SIZE (in bytes) and alignment ALIGN (in bits).
   This macro exists to properly support languages like C++ which do not
   have common data.  */

#define ASM_OUTPUT_ALIGNED_BSS(FILE, DECL, NAME, SIZE, ALIGN)		\
  pa_asm_output_aligned_bss (FILE, NAME, SIZE, ALIGN)
  
/* This says how to output an assembler line to define a global common symbol
   with size SIZE (in bytes) and alignment ALIGN (in bits).  */

#define ASM_OUTPUT_ALIGNED_COMMON(FILE, NAME, SIZE, ALIGN)  		\
  pa_asm_output_aligned_common (FILE, NAME, SIZE, ALIGN)

/* This says how to output an assembler line to define a local common symbol
   with size SIZE (in bytes) and alignment ALIGN (in bits).  This macro
   controls how the assembler definitions of uninitialized static variables
   are output.  */

#define ASM_OUTPUT_ALIGNED_LOCAL(FILE, NAME, SIZE, ALIGN)		\
  pa_asm_output_aligned_local (FILE, NAME, SIZE, ALIGN)
  
/* All HP assemblers use "!" to separate logical lines.  */
#define IS_ASM_LOGICAL_LINE_SEPARATOR(C, STR) ((C) == '!')

/* Print operand X (an rtx) in assembler syntax to file FILE.
   CODE is a letter or dot (`z' in `%z0') or 0 if no letter was specified.
   For `%' followed by punctuation, CODE is the punctuation and X is null.

   On the HP-PA, the CODE can be `r', meaning this is a register-only operand
   and an immediate zero should be represented as `r0'.

   Several % codes are defined:
   O an operation
   C compare conditions
   N extract conditions
   M modifier to handle preincrement addressing for memory refs.
   F modifier to handle preincrement addressing for fp memory refs */

#define PRINT_OPERAND(FILE, X, CODE) pa_print_operand (FILE, X, CODE)


/* Print a memory address as an operand to reference that memory location.  */

#define PRINT_OPERAND_ADDRESS(FILE, ADDR)  \
{ rtx addr = ADDR;							\
  switch (GET_CODE (addr))						\
    {									\
    case REG:								\
      fprintf (FILE, "0(%s)", reg_names [REGNO (addr)]);		\
      break;								\
    case PLUS:								\
      gcc_assert (GET_CODE (XEXP (addr, 1)) == CONST_INT);		\
      fprintf (FILE, "%d(%s)", (int)INTVAL (XEXP (addr, 1)),		\
	       reg_names [REGNO (XEXP (addr, 0))]);			\
      break;								\
    case LO_SUM:							\
      if (!symbolic_operand (XEXP (addr, 1), VOIDmode))			\
	fputs ("R'", FILE);						\
      else if (flag_pic == 0)						\
	fputs ("RR'", FILE);						\
      else								\
	fputs ("RT'", FILE);						\
      pa_output_global_address (FILE, XEXP (addr, 1), 0);		\
      fputs ("(", FILE);						\
      output_operand (XEXP (addr, 0), 0);				\
      fputs (")", FILE);						\
      break;								\
    case CONST_INT:							\
      fprintf (FILE, HOST_WIDE_INT_PRINT_DEC "(%%r0)", INTVAL (addr));	\
      break;								\
    default:								\
      output_addr_const (FILE, addr);					\
    }}


/* Find the return address associated with the frame given by
   FRAMEADDR.  */
#define RETURN_ADDR_RTX(COUNT, FRAMEADDR)				 \
  (pa_return_addr_rtx (COUNT, FRAMEADDR))

/* Used to mask out junk bits from the return address, such as
   processor state, interrupt status, condition codes and the like.  */
#define MASK_RETURN_ADDR						\
  /* The privilege level is in the two low order bits, mask em out	\
     of the return address.  */						\
  (GEN_INT (-4))

/* We need a libcall to canonicalize function pointers on TARGET_ELF32.  */
#define CANONICALIZE_FUNCPTR_FOR_COMPARE_LIBCALL \
  "__canonicalize_funcptr_for_compare"

#ifdef HAVE_AS_TLS
#undef TARGET_HAVE_TLS
#define TARGET_HAVE_TLS true
#endif

/* The maximum offset in bytes for a PA 1.X pc-relative call to the
   head of the preceding stub table.  A long branch stub is two or three
   instructions for non-PIC and PIC, respectively.  Import stubs are
   seven and five instructions for HP-UX and ELF targets, respectively.
   The default stub group size for ELF targets is 217856 bytes.
   FIXME: We need an option to set the maximum offset.  */  
#define MAX_PCREL17F_OFFSET (TARGET_HPUX ? 198164 : 217856)

#define NEED_INDICATE_EXEC_STACK 0

/* Output default function prologue for hpux.  */
#define TARGET_ASM_FUNCTION_PROLOGUE pa_output_function_prologue

/* Definitions for AMD x86-64 running FreeBSD with ELF format
   Copyright (C) 2002-2023 Free Software Foundation, Inc.
   Contributed by David O'Brien <obrien@FreeBSD.org>

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


#define SUBTARGET_EXTRA_SPECS \
  { "fbsd_dynamic_linker", FBSD_DYNAMIC_LINKER }

#undef CC1_SPEC
#define CC1_SPEC "%(cc1_cpu) %{profile:-p}"

/* Provide a LINK_SPEC appropriate for the FreeBSD/x86-64 ELF target.
   This is a copy of LINK_SPEC from <i386/freebsd.h> tweaked for
   the x86-64 target.  */

#undef	LINK_SPEC
#define LINK_SPEC "\
  %{m32:-m elf_i386_fbsd}%{!m32:-m elf_x86_64_fbsd} \
  %{p:%nconsider using '-pg' instead of '-p' with gprof(1)} \
  " FBSD_LINK_PG_NOTE " \
  %{v:-V} \
  %{assert*} %{R*} %{rpath*} %{defsym*} \
  %{shared:-Bshareable %{h*} %{soname*}} \
    %{!shared: \
      %{!static: \
        %{rdynamic:-export-dynamic} \
	-dynamic-linker %(fbsd_dynamic_linker) } \
    %{static:-Bstatic}} \
  %{symbolic:-Bsymbolic}"

#undef	MULTILIB_DEFAULTS
#define MULTILIB_DEFAULTS { "m64" }

/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <platform_def.h>

OUTPUT_FORMAT(PLATFORM_LINKER_FORMAT)
OUTPUT_ARCH(PLATFORM_LINKER_ARCH)
ENTRY(bl31_entrypoint)


MEMORY {
    RAM (rwx): ORIGIN = BL31_BASE, LENGTH = BL31_LIMIT
    RAM2 (rwx): ORIGIN = TZRAM2_BASE, LENGTH = TZRAM2_SIZE
}


SECTIONS
{
    . = BL31_BASE;
    ASSERT(. == ALIGN(4096),
           "BL31_BASE address is not aligned on a page boundary.")

    ro . : {
        __RO_START__ = .;
        *bl31_entrypoint.o(.text*)
        *(.text*)
        *(.rodata*)

        /* Ensure 8-byte alignment for descriptors and ensure inclusion */
        . = ALIGN(8);
        __RT_SVC_DESCS_START__ = .;
        KEEP(*(rt_svc_descs))
        __RT_SVC_DESCS_END__ = .;

        /*
         * Ensure 8-byte alignment for cpu_ops so that its fields are also
         * aligned. Also ensure cpu_ops inclusion.
         */
        . = ALIGN(8);
        __CPU_OPS_START__ = .;
        KEEP(*(cpu_ops))
        __CPU_OPS_END__ = .;

        *(.vectors)
        __RO_END_UNALIGNED__ = .;
        /*
         * Memory page(s) mapped to this section will be marked as read-only,
         * executable.  No RW data from the next section must creep in.
         * Ensure the rest of the current memory page is unused.
         */
        . = NEXT(4096);
        __RO_END__ = .;
    } >RAM

    ASSERT(__CPU_OPS_END__ > __CPU_OPS_START__,
           "cpu_ops not defined for this platform.")

    .data . : {
        __DATA_START__ = .;
        *(.data*)
        __DATA_END__ = .;
    } >RAM

#ifdef BL31_PROGBITS_LIMIT
    ASSERT(. <= BL31_PROGBITS_LIMIT, "BL3-1 progbits has exceeded its limit.")
#endif

    stacks (NOLOAD) : {
        __STACKS_START__ = .;
        *(tzfw_normal_stacks)
        __STACKS_END__ = .;
    } >RAM

    /*
     * The .bss section gets initialised to 0 at runtime.
     * Its base address must be 16-byte aligned.
     */
    .bss : ALIGN(16) {
        __BSS_START__ = .;
        *(.bss*)
        *(COMMON)
        __BSS_END__ = .;
    } >RAM

    ASSERT(. <= BL31_LIMIT, "BL3-1 image has exceeded its limit.")

    /*
     * The xlat_table section is for full, aligned page tables (4K).
     * Removing them from .bss avoids forcing 4K alignment on
     * the .bss section and eliminates the unecessary zero init
     */
    xlat_table (NOLOAD) : {
        *(xlat_table)
    } >RAM2

    /*
     * The base address of the coherent memory section must be page-aligned (4K)
     * to guarantee that the coherent data are stored on their own pages and
     * are not mixed with normal data.  This is required to set up the correct
     * memory attributes for the coherent data page tables.
     */
    coherent_ram (NOLOAD) : ALIGN(4096) {
        __COHERENT_RAM_START__ = .;
        *(tzfw_coherent_mem)
        __COHERENT_RAM_END_UNALIGNED__ = .;
        /*
         * Memory page(s) mapped to this section will be marked
         * as device memory.  No other unexpected data must creep in.
         * Ensure the rest of the current memory page is unused.
         */
        . = NEXT(4096);
        __COHERENT_RAM_END__ = .;
    } >RAM2

    __BL31_END__ = .;

    __BSS_SIZE__ = SIZEOF(.bss);
    __COHERENT_RAM_UNALIGNED_SIZE__ =
        __COHERENT_RAM_END_UNALIGNED__ - __COHERENT_RAM_START__;

    ASSERT(. <= TZRAM2_LIMIT, "TZRAM2 image has exceeded its limit.")
}

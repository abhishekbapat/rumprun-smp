/*-
 * Copyright (c) 2014 Antti Kantee.  All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <hw/types.h>
#include <hw/multiboot.h>
#include <hw/kernel.h>

#include <bmk-core/core.h>
#include <bmk-core/pgalloc.h>
#include <bmk-core/printf.h>
#include <bmk-core/string.h>

#include <bmk-pcpu/pcpu.h>

#define MEMSTART 0x100000

static uint32_t
parsemem(struct multiboot_tag_mmap *tag)
{
    uint32_t mmap_found = 0;
    multiboot_memory_map_t *mmap;
    unsigned long osend;
    extern char _end[];

    /*
     * Look for our memory.  We assume it's just in one chunk
     * starting at MEMSTART.
     */
    for (mmap = tag->entries;
         (multiboot_uint8_t *)mmap < (multiboot_uint8_t *)tag + tag->size;
         mmap = (multiboot_memory_map_t *)((unsigned long)mmap + ((struct multiboot_tag_mmap *)tag)->entry_size))
    {
        if (mmap->addr == MEMSTART && mmap->type == MULTIBOOT_MEMORY_AVAILABLE)
        {
            mmap_found++;
            bmk_printf("multiboot2 memory chunk found\n");
            break;
        }
    }

    if (mmap_found == 0)
        bmk_platform_halt("multiboot2 memory chunk not found");

    osend = bmk_round_page((unsigned long)_end);
    bmk_assert(osend > mmap->addr && osend < mmap->addr + mmap->len);

    bmk_pgalloc_loadmem(osend, mmap->addr + mmap->len);

    bmk_memsize = mmap->addr + mmap->len - osend;

    return 0;
}

char multiboot_cmdline[BMK_MULTIBOOT_CMDLINE_SIZE];

void multiboot(unsigned long addr)
{
    uint32_t module_count = 0;
    uint32_t cmdline_count = 0;
    uint32_t memory_parse_count = 0;
    struct multiboot_tag *tag;
    unsigned long cmdlinelen;
    char *cmdline = NULL,
         *mbm_name;
    bmk_core_init(BMK_THREAD_STACK_PAGE_ORDER);
    for (tag = (struct multiboot_tag *)(addr + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag *)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7)))
    {
        switch (tag->type)
        {
        case MULTIBOOT_TAG_TYPE_MODULE:
            if (module_count == 0)
            {
                struct multiboot_tag_module *mtm = (struct multiboot_tag_module *)tag;
                module_count++;
                mbm_name = (char *)mtm->cmdline;
                cmdline = (char *)(uintptr_t)mtm->mod_start;
                cmdlinelen = mtm->mod_end - mtm->mod_start;
                if (cmdlinelen >= (BMK_MULTIBOOT_CMDLINE_SIZE - 1))
                    bmk_platform_halt("command line too long, "
                                      "increase BMK_MULTIBOOT_CMDLINE_SIZE");

                bmk_printf("multiboot2: Using configuration from %s\n",
                           mbm_name ? mbm_name : "(unnamed module)");
                bmk_memcpy(multiboot_cmdline, cmdline, cmdlinelen);
                multiboot_cmdline[cmdlinelen] = 0;
            }
            break;
        case MULTIBOOT_TAG_TYPE_CMDLINE:
            if (cmdline_count == 0 && module_count == 0)
            {
                cmdline_count++;
                struct multiboot_tag_string *mts = (struct multiboot_tag_string *)tag;
                cmdline = (char *)mts->string;
                cmdlinelen = bmk_strlen(cmdline);
                if (cmdlinelen >= BMK_MULTIBOOT_CMDLINE_SIZE)
                    bmk_platform_halt("command line too long, "
                                      "increase BMK_MULTIBOOT_CMDLINE_SIZE");
                bmk_strcpy(multiboot_cmdline, cmdline);
            }
            break;
        case MULTIBOOT_TAG_TYPE_MMAP:
            if (memory_parse_count == 0)
            {
                if (parsemem((struct multiboot_tag_mmap *)tag) == 0)
                    memory_parse_count++;
            }
            break;
        default:
            break;
        }
    }

    if (cmdline == NULL)
        multiboot_cmdline[0] = 0;

    if (memory_parse_count == 0)
        bmk_platform_halt("multiboot memory parse failed");
}
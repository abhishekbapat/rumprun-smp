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

static uint32_t
parsemem(struct multiboot_tag_mmap *tag)
{
    uint32_t mmap_found = 0;
    multiboot_memory_map_t *mmap;
    unsigned long osend;
    extern char _end[];
    uint64_t max_len = 0;
    multiboot_memory_map_t *mmap_considered;

    /*
     * Look for our memory.  We assume it's just in one chunk
     * starting at MEMSTART.
     */
    for (mmap = tag->entries;
         (multiboot_uint8_t *)mmap < (multiboot_uint8_t *)tag + tag->size;
         mmap = (multiboot_memory_map_t *)((unsigned long)mmap + ((struct multiboot_tag_mmap *)tag)->entry_size))
    {
        if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE)
        {
            mmap_found++;
            if (mmap->len > max_len)
            {
                mmap_considered = mmap;
                max_len = (uint64_t)mmap->len;
            }
        }
    }

    if (mmap_found == 0)
        bmk_platform_halt("multiboot2 memory chunk not found\n");

    if (mmap_considered->addr < (uint64_t)_end)
    {
        osend = bmk_round_page((unsigned long)_end);
        bmk_assert(osend > mmap_considered->addr && osend < mmap_considered->addr + mmap_considered->len);
        bmk_pgalloc_loadmem(osend, mmap_considered->addr + mmap_considered->len);
        bmk_memsize = mmap_considered->addr + mmap_considered->len - osend;
    }
    else
    {
        unsigned long addr = bmk_round_page((unsigned long)mmap_considered->addr);
        bmk_pgalloc_loadmem(addr, mmap_considered->addr + mmap_considered->len);
        bmk_memsize = mmap_considered->addr + mmap_considered->len - addr;
    }

    return 0;
}

char multiboot_cmdline[BMK_MULTIBOOT_CMDLINE_SIZE];

int multiboot(unsigned long addr)
{
    int uefi = 0;
    uint32_t module_count = 0;
    uint32_t cmdline_count = 0;
    uint32_t memory_info_count = 0;
    uint32_t memory_parse_count = 0;
    struct multiboot_tag *tag;
    unsigned long cmdlinelen;
    char *cmdline = NULL,
         *mbm_name;
    //unsigned total_size;
    bmk_core_init(BMK_THREAD_STACK_PAGE_ORDER);
    //total_size = *(unsigned *)addr;
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
                //bmk_platform_halt("test 1");
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
                //bmk_platform_halt("test 2");
                bmk_strcpy(multiboot_cmdline, cmdline);
            }
            break;
        case MULTIBOOT_TAG_TYPE_MMAP:
            if (memory_parse_count == 0)
            {
                //bmk_platform_halt("test 3");
                if (parsemem((struct multiboot_tag_mmap *)tag) == 0)
                    memory_parse_count++;
            }
            break;
        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
            if (memory_info_count == 0)
            {
                memory_info_count++;
                // bmk_platform_halt("test 4");
            }
            break;
        case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
        {
            struct multiboot_tag_string *mts = (struct multiboot_tag_string *)tag;
            bmk_printf("Bootloader name: %s\n", mts->string);
            break;
        }
        case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
        {
            struct multiboot_tag_framebuffer *tagfb = (struct multiboot_tag_framebuffer *)tag;
            unsigned int *fb_addr = (unsigned int *)tagfb->common.framebuffer_addr;
            if (((uint64_t)(fb_addr)) != 0xb8000) // Means UEFI
                uefi = 1;
            break;
        }
        default:
            break;
        }
    }

    if (cmdline == NULL)
        multiboot_cmdline[0] = 0;

    if (memory_info_count == 0 && memory_parse_count == 0)
        bmk_platform_halt("multiboot memory info not available\n");

    if (memory_parse_count == 0)
        bmk_platform_halt("multiboot memory parse failed");
    return uefi;
}
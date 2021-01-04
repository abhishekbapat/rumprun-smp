#include <bmk-pcpu/pcpu.h>

void __attribute__((noreturn)) splfatal(void);

static inline uint8_t
inb(uint16_t port)
{
        uint8_t rv;

        __asm__ __volatile__("inb %1, %0" : "=a"(rv) : "d"(port));

        return rv;
}

static inline uint32_t
inl(uint16_t port)
{
        uint32_t rv;

        __asm__ __volatile__("inl %1, %0" : "=a"(rv) : "d"(port));

        return rv;
}

static inline void
outb(uint16_t port, uint8_t value)
{

        __asm__ __volatile__("outb %0, %1" :: "a"(value), "d"(port));
}

static inline void
outl(uint16_t port, uint32_t value)
{

        __asm__ __volatile__("outl %0, %1" :: "a"(value), "d"(port));
}

static inline void
splhigh(void)
{

	__asm__ __volatile__("cli");
	if (bmk_add_cpu(spldepth, 1))
		splfatal();
}

static inline void
spl0(void)
{

	if (bmk_sub_cpu(spldepth, 1))
		__asm__ __volatile__("sti");
}

static inline void
hlt(void)
{

	__asm__ __volatile__("hlt");
}

static inline uint64_t
rdtsc(void)
{
	uint64_t val;
	unsigned long eax, edx;

	__asm__ __volatile__("rdtsc" : "=a"(eax), "=d"(edx));
	val = ((uint64_t)edx<<32)|(eax);
	return val;
}

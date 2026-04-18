/*
 * Charset probe — B2.1 linker script.
 *
 * Same MEMORY model as the default C64 PRG (`mos-platform/c64/lib/link.ld`: `ram` @ `$0801`,
 * `LENGTH 0xC7FF`), but asserts that no allocated data ends in `$3000–$37FF`.
 *
 * Contiguous llvm-mos sections grow upward from `$0801`; for typical probe sizes everything
 * stays below `$3000`. If the binary grows past `$3000`, the link fails unless layout is
 * split (future work) — same safety as `charset_probe.ld`, with a map that matches the
 * production link script shape.
 */

__basic_zp_start = 0x0002;
__basic_zp_end = 0x0090;

MEMORY {
    ram (rw) : ORIGIN = 0x0801, LENGTH = 0xC7FF
}

INCLUDE commodore.ld

INPUT(unmap-basic.o)

__stack = 0xD000;

ASSERT(__heap_start <= 0x3000, "B2.1: sections must not overlap charset RAM $3000-$37FF");

OUTPUT_FORMAT {
    SHORT(ORIGIN(ram))
    TRIM(ram)
}

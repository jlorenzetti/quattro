/*
 * C64 charset probe — linker script.
 *
 * Reserves $3000–$37FF for VIC-II character data (no sections placed there).
 * Limits the loadable segment to $0801–$2FFF so the runtime cannot overlap
 * the charset region used by $D018.
 */

__basic_zp_start = 0x0002;
__basic_zp_end = 0x0090;

MEMORY {
    ram (rw) : ORIGIN = 0x0801, LENGTH = 0x27FF
}

INCLUDE commodore.ld

INPUT(unmap-basic.o)

__stack = 0xD000;

OUTPUT_FORMAT {
    SHORT(ORIGIN(ram))
    TRIM(ram)
}

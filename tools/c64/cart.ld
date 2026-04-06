/*
 * C64 16 KB cartridge ROM ($8000–$BFFF). LMA: .cart_header → .zp.data → .cart_boot → .text → .rodata.
 * Omit unmap-basic.o (PRG banking). See docs/notes/c64-cartridge.md.
 */

__basic_zp_start = 0x0002;
__basic_zp_end = 0x0090;

__rc0 = __basic_zp_start;
INCLUDE imag-regs.ld

MEMORY {
    zp (rw) : ORIGIN = __rc31 + 1, LENGTH = __basic_zp_end - (__rc31 + 1)
    rom (rx) : ORIGIN = 0x8000, LENGTH = 0x4000
    ram (rw) : ORIGIN = 0x0801, LENGTH = 0x77FF
}

REGION_ALIAS("c_readonly", rom)
REGION_ALIAS("c_writeable", ram)

SECTIONS {
    .cart_header : {
        KEEP(*(.cart_header))
    } > rom

    INCLUDE zp.ld

    .cart_boot : {
        KEEP(*(.cart_boot))
    } > rom

    INCLUDE text.ld
    INCLUDE rodata.ld
    INCLUDE data.ld
    INCLUDE bss.ld
    INCLUDE noinit.ld
}

/* lld can set __zp_data_load_start from LOADADDR(.zp.data) to .cart_boot LMA; ZP image follows the 9-byte header. */
__zp_data_load_start = LOADADDR(.cart_header) + SIZEOF(.cart_header);
ASSERT(
    LOADADDR(.zp.data) == LOADADDR(.cart_header) + SIZEOF(.cart_header),
    "cart: .zp.data LMA must follow .cart_header"
);

/* ROM image must not exceed 16 KB at $8000. */
ASSERT(LOADADDR(.rodata) + SIZEOF(.rodata) <= 0xC000, "rom overflow");

__stack = 0xD000;

OUTPUT_FORMAT { FULL(rom) }

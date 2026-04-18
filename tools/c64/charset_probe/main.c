/**
 * @file main.c
 * @brief Minimal C64 charset probe: VIC text mode, custom charset at $3000, few test glyphs.
 */

#include <stdint.h>

/** 6510 port direction and data (RAM/ROM/I/O banking). */
#define C64_6510_DDR (*(volatile unsigned char *)0x00)
#define C64_6510_PORT (*(volatile unsigned char *)0x01)
/** CIA2: port A (VIC bank lines) and data direction. */
#define CIA2_DDRA (*(volatile unsigned char *)0xDD02)
#define CIA2_PRA (*(volatile unsigned char *)0xDD00)
/** VIC-II: control and screen/charset pointers for 40×25 @ `$0400`. */
#define VIC_CTRL1 (*(volatile unsigned char *)0xD011)
#define VIC_CTRL2 (*(volatile unsigned char *)0xD016)
#define VIC_MEM_PTR (*(volatile unsigned char *)0xD018)

/** C64 screen and color RAM (default layout). */
#define SCREEN_RAM ((volatile unsigned char *)0x0400)
#define COLOR_RAM ((volatile unsigned char *)0xD800)

/** Target address for 2K charset (VIC bank 0, aligned with $D018). */
#define CHARSET_RAM ((volatile unsigned char *)0x3000)

/** Screen columns. */
#define COLS 40

/**
 * @brief Computes `$D018` for a given screen base and charset base within the VIC 16K bank.
 *
 * On VIC-II, lower nybble = character base in 1KB steps; upper nybble = video matrix in 1KB steps.
 */
static unsigned char vic_mem_ptr_for_charset(uintptr_t screen_base, uintptr_t charset_base) {
    const unsigned char screen_idx = (unsigned char)((screen_base >> 10) & 0x0Fu);
    const unsigned char char_idx = (unsigned char)((charset_base >> 10) & 0x0Fu);
    return (unsigned char)((screen_idx << 4) | char_idx);
}

_Static_assert(((1u << 4) | (0x3000u >> 10)) == 0x1Cu, "D018 for screen $0400 + charset $3000");

/**
 * @brief Aligns VIC with Quattro baseline: 40×25 @ `$0400`, bank 0, then `cli`.
 */
static void vic_init_text_0400_baseline(void) {
    C64_6510_DDR = 0x2Fu;
    C64_6510_PORT = 0x37u;

    VIC_CTRL1 = 0x9Bu;
    VIC_CTRL2 = 0x08u;

    CIA2_DDRA = (unsigned char)(CIA2_DDRA | 0x03u);
    CIA2_PRA = (unsigned char)((CIA2_PRA & (unsigned char)~0x03u) | 0x03u);

    /* ROM charset baseline (same as `src/platform/c64/video.c`). */
    VIC_MEM_PTR = 0x17u;

    __asm__ volatile("cli" ::: "memory");
}

/**
 * @brief Clears screen to spaces and color RAM to white-on-blue (visible, readable).
 */
static void clear_screen(void) {
    for (unsigned i = 0; i < 25U * COLS; ++i) {
        SCREEN_RAM[i] = 0x20u;
        COLOR_RAM[i] = 0x0Eu; /* light blue */
    }
}

/**
 * @brief Writes four test glyphs (screen codes 0–3) at row 0, columns 0–3.
 */
static void draw_test_glyphs(void) {
    for (unsigned c = 0; c < 4u; ++c) {
        SCREEN_RAM[c] = (unsigned char)c;
        COLOR_RAM[c] = 0x01u; /* white */
    }
}

/** Four distinct 8×8 patterns for screen codes 0–3. */
static const unsigned char k_test_glyphs[4][8] = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55},
    {0x18, 0x3C, 0x7E, 0xFF, 0xFF, 0x7E, 0x3C, 0x18},
    {0x81, 0xC3, 0xE7, 0xFF, 0xFF, 0xE7, 0xC3, 0x81},
};

/**
 * @brief Installs charset into `$3000–$37FF`: zeros, then first four glyphs.
 */
static void install_charset_at_3000(void) {
    volatile unsigned char *p = CHARSET_RAM;
    for (unsigned i = 0; i < 2048u; ++i) {
        p[i] = 0u;
    }
    for (unsigned ch = 0; ch < 4u; ++ch) {
        for (unsigned b = 0; b < 8u; ++b) {
            p[ch * 8u + b] = k_test_glyphs[ch][b];
        }
    }
}

/**
 * @brief Entry: init VIC, fill charset RAM, point `$D018` at `$3000`, show glyphs, halt.
 */
int main(void) {
    vic_init_text_0400_baseline();
    clear_screen();
    install_charset_at_3000();

    VIC_MEM_PTR = vic_mem_ptr_for_charset(0x0400u, 0x3000u);
    draw_test_glyphs();

    for (;;) {
        __asm__ volatile("" ::: "memory");
    }
}

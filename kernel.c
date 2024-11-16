#include "kernel.h"

// Terminal state
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;
static bool cursor_enabled = true;

// Screen state
typedef struct {
    size_t row;
    size_t column;
    uint8_t color;
    uint16_t buffer[VGA_WIDTH * VGA_HEIGHT];
} screen_t;

static screen_t screens[MAX_SCREENS];
static uint8_t current_screen = 0;

// Utility Functions
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void sleep(int ticks) {
    for (int i = 0; i < ticks * 10000; i++) {
        asm volatile("nop");
    }
}

// VGA Functions
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

// Cursor Management
void enable_cursor(void) {
    outb(CURSOR_PORT_COMMAND, 0x0A);
    outb(CURSOR_PORT_DATA, (inb(CURSOR_PORT_DATA) & 0xC0) | 0);
    outb(CURSOR_PORT_COMMAND, 0x0B);
    outb(CURSOR_PORT_DATA, (inb(CURSOR_PORT_DATA) & 0xE0) | 15);
    cursor_enabled = true;
}

void disable_cursor(void) {
    outb(CURSOR_PORT_COMMAND, 0x0A);
    outb(CURSOR_PORT_DATA, 0x20);
    cursor_enabled = false;
}

void update_cursor(void) {
    if (!cursor_enabled) return;
    
    uint16_t pos = terminal_row * VGA_WIDTH + terminal_column;
    outb(CURSOR_PORT_COMMAND, 0x0F);
    outb(CURSOR_PORT_DATA, (uint8_t)(pos & 0xFF));
    outb(CURSOR_PORT_COMMAND, 0x0E);
    outb(CURSOR_PORT_DATA, (uint8_t)((pos >> 8) & 0xFF));
}

// Screen Management
void clear_screen(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_putentryat(' ', terminal_color, x, y);
        }
    }
    terminal_row = 0;
    terminal_column = 0;
    update_cursor();
}

void terminal_scroll(void) {
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t current_index = y * VGA_WIDTH + x;
            const size_t next_index = (y + 1) * VGA_WIDTH + x;
            terminal_buffer[current_index] = terminal_buffer[next_index];
        }
    }
    
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
    
    terminal_row = VGA_HEIGHT - 1;
    terminal_column = 0;
    update_cursor();
}

// Terminal Output Functions
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
        }
    } else {
        terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
        if (++terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) {
                terminal_scroll();
            }
        }
    }
    update_cursor();
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_print_colored(const char* str, enum vga_color fg, enum vga_color bg) {
    uint8_t old_color = terminal_color;
    terminal_color = vga_entry_color(fg, bg);
    terminal_writestring(str);
    terminal_color = old_color;
}

// Animation Effects
void matrix_effect(void) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        for (size_t y = 0; y < VGA_HEIGHT; y++) {
            terminal_putentryat('4', vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK), x, y);
            terminal_putentryat('2', vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK), (x + 1) % VGA_WIDTH, y);
            update_cursor();
            sleep(1);
        }
    }
    clear_screen();
}

void type_text(const char* text, enum vga_color color, int delay) {
    uint8_t old_color = terminal_color;
    terminal_color = vga_entry_color(color, VGA_COLOR_BLACK);
    
    for (size_t i = 0; text[i] != '\0'; i++) {
        terminal_putchar(text[i]);
        update_cursor();
        sleep(delay);
    }
    
    terminal_color = old_color;
}

void fade_in_text(const char* text, size_t row, size_t col) {
    enum vga_color colors[] = {
        VGA_COLOR_DARK_GREY,
        VGA_COLOR_LIGHT_GREY,
        VGA_COLOR_WHITE
    };
    
    for (size_t i = 0; i < 3; i++) {
        terminal_row = row;
        terminal_column = col;
        terminal_print_colored(text, colors[i], VGA_COLOR_BLACK);
        sleep(100);
    }
}

void show_loading_bar(void) {
    terminal_row = VGA_HEIGHT - 2;
    terminal_column = 0;
    terminal_print_colored("[", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    
    for (size_t i = 0; i < 50; i++) {
        terminal_print_colored("=", VGA_COLOR_CYAN, VGA_COLOR_BLACK);
        sleep(20);
    }
    
    terminal_print_colored("]", VGA_COLOR_WHITE, VGA_COLOR_BLACK);
}

// Screen Switching
void switch_screen(uint8_t screen_num) {
    if (screen_num >= MAX_SCREENS) return;
    
    // Save current screen
    screens[current_screen].row = terminal_row;
    screens[current_screen].column = terminal_column;
    screens[current_screen].color = terminal_color;
    
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        screens[current_screen].buffer[i] = terminal_buffer[i];
    }
    
    // Load new screen
    terminal_row = screens[screen_num].row;
    terminal_column = screens[screen_num].column;
    terminal_color = screens[screen_num].color;
    
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        terminal_buffer[i] = screens[screen_num].buffer[i];
    }
    
    current_screen = screen_num;
    update_cursor();
}

void init_screens(void) {
    for (uint8_t i = 0; i < MAX_SCREENS; i++) {
        screens[i].row = 0;
        screens[i].column = 0;
        screens[i].color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        
        for (size_t j = 0; j < VGA_WIDTH * VGA_HEIGHT; j++) {
            screens[i].buffer[j] = vga_entry(' ', screens[i].color);
        }
    }
}

void terminal_init(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) VGA_MEMORY;
    
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
    
    enable_cursor();
}

void kernel_main(void) {
    // Initialize systems
    terminal_init();
    init_screens();
    
    // Start with Matrix-style effect
    matrix_effect();
    sleep(200);
    
    // Main welcome message with typing effect
    terminal_row = 5;
    terminal_column = 15;
    type_text("Welcome to the Kernel From Scratch (KFS) Project!\n\n", VGA_COLOR_CYAN, 30);
    
    // Fade in the 42 reference
    terminal_row = 8;
    terminal_column = 20;
    fade_in_text("The Answer to KFS is 42!\n", 8, 20);
    sleep(200);
    
    // Type out some puns with different colors
    terminal_row = 11;
    terminal_column = 10;
    type_text("Don't Panic! This kernel is mostly harmless...\n", VGA_COLOR_GREEN, 20);
    
    terminal_row = 13;
    terminal_column = 10;
    type_text("In space, no one can hear you segfault!\n", VGA_COLOR_MAGENTA, 20);
    
    // Show a "loading" progress bar
    terminal_row = 15;
    terminal_column = 15;
    type_text("Loading quantum bits...\n", VGA_COLOR_LIGHT_BLUE, 30);
    show_loading_bar();
    
    // Final system info
    terminal_row = 18;
    terminal_column = 20;
    fade_in_text("System Status: All 42 cores active\n", 18, 20);
    
    terminal_row = 20;
    terminal_column = 20;
    type_text("Press any key to begin your journey...\n", VGA_COLOR_LIGHT_GREY, 20);
    
    // Reset cursor to a sensible position
    terminal_row = 23;
    terminal_column = 0;
    update_cursor();
}

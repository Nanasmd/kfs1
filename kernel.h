#ifndef KERNEL_H
#define KERNEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Constantes pour le mode texte VGA
 * Implémentation des bonus:
 * - Support des couleurs
 * - Support du curseur
 * - Support du défilement
 * - Support de plusieurs écrans
 */
#define VGA_WIDTH 80          // Largeur standard du mode texte VGA
#define VGA_HEIGHT 25         // Hauteur standard du mode texte VGA
#define VGA_MEMORY 0xB8000    // Adresse mémoire du buffer VGA
#define MAX_SCREENS 4         // Nombre d'écrans virtuels (bonus)

// Ports pour le contrôle du curseur matériel
#define CURSOR_PORT_COMMAND 0x3D4
#define CURSOR_PORT_DATA 0x3D5

/*
 * Énumération des couleurs VGA
 * Utilisé pour le bonus de support des couleurs
 * Permet des combinaisons de couleurs de texte et de fond
 */
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

// Prototypes des fonctions principales

/*
 * Fonctions d'initialisation et de contrôle du terminal
 * Implémente les fonctionnalités de base requises par KFS1
 */
void terminal_init(void);
void terminal_setcolor(uint8_t color);
void terminal_writestring(const char* data);

/*
 * Fonctions pour le support des bonus
 * - Gestion du curseur
 * - Effets de couleur
 * - Animations
 * - Écrans multiples
 */
void enable_cursor(void);
void disable_cursor(void);
void update_cursor(void);
void terminal_print_colored(const char* str, enum vga_color fg, enum vga_color bg);
void switch_screen(uint8_t screen_num);
void init_screens(void);

/*
 * Fonctions d'animation (bonus supplémentaire)
 * Ajoute des effets visuels pour une meilleure présentation
 */
void matrix_effect(void);
void type_text(const char* text, enum vga_color color, int delay);
void fade_in_text(const char* text, size_t row, size_t col);
void show_loading_bar(void);

#endif

/*
 * Script de linkage personnalisé pour KFS1
 * Nécessaire car on ne peut pas utiliser le linker du système hôte
 */

/* Point d'entrée défini dans boot.s */
ENTRY(_start)

SECTIONS
{
    /*
     * On commence à 2M pour la compatibilité UEFI
     * Plus sûr que l'ancien standard de 1M
     */
    . = 2M;

    /* Section .text pour le code */
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)    /* En-tête multiboot en premier */
        *(.text)         /* Code du noyau */
    }

    /* Section .rodata pour les données en lecture seule */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    /* Section .data pour les données initialisées */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    /* Section .bss pour les données non initialisées et la stack */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }
}

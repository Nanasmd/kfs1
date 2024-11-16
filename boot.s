/*
 * Code de démarrage pour KFS1
 * Implémente l'en-tête multiboot et initialise le noyau
 */

# Configuration multiboot pour GRUB
.set ALIGN,    1<<0             # Alignement sur les limites de page
.set MEMINFO,  1<<1             # Fournir une carte mémoire
.set FLAGS,    ALIGN | MEMINFO  # Champs de drapeaux
.set MAGIC,    0x1BADB002       # Magic number pour multiboot
.set CHECKSUM, -(MAGIC + FLAGS) # Checksum requis

# Section multiboot pour GRUB
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/*
 * Section BSS - Stack
 * Taille augmentée à 32Ko pour plus de stabilité
 * Important pour les animations et les écrans multiples
 */
.section .bss
.align 16
stack_bottom:
    .skip 32768                # Stack de 32Ko
stack_top:

/*
 * Point d'entrée du noyau
 * Configure la stack et appelle le kernel_main
 */
.section .text
.global _start
.type _start, @function

_start:
    mov $stack_top, %esp      # Initialise le pointeur de stack
    push %ebx                 # Sauvegarde les infos multiboot
    call kernel_main          # Appelle notre fonction principale

    # Boucle infinie si kernel_main retourne
    cli                       
1:  hlt
    jmp 1b

.size _start, . - _start

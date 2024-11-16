# Fichiers de sortie
OUTPUT_BIN = myos.bin
ISO = myos.iso

# Compilateur et assembleur pour architecture i386
AS = i686-elf-as
CC = i686-elf-gcc

# Flags de compilation conformes aux exigences KFS1
# -fno-builtin : Pas de fonctions builtin
# -fno-exceptions : Pas d'exceptions
# -fno-stack-protector : Pas de protection de stack
# -fno-rtti : Pas de RTTI
# -nostdlib : Pas de stdlib
# -nodefaultlibs : Pas de libs par défaut
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra \
         -fno-builtin -fno-exceptions -fno-stack-protector \
         -fno-rtti -nostdlib -nodefaultlibs

# Flags de linking avec notre script personnalisé
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib

# Fichiers source
SOURCES = kernel.c
HEADERS = kernel.h

# Cible par défaut
all: $(ISO)

# Compilation du code assembleur
boot.o: boot.s
	$(AS) boot.s -o boot.o

# Compilation du code C
kernel.o: $(SOURCES) $(HEADERS)
	$(CC) -c $(SOURCES) -o kernel.o $(CFLAGS)

# Linking pour créer le noyau
$(OUTPUT_BIN): boot.o kernel.o
	$(CC) $(LDFLAGS) -o $(OUTPUT_BIN) boot.o kernel.o -lgcc

# Vérification multiboot
multiboot-check: $(OUTPUT_BIN)
	@if grub-file --is-x86-multiboot $(OUTPUT_BIN); then \
		echo "Multiboot confirmé"; \
	else \
		echo "Erreur: non compatible multiboot"; \
		exit 1; \
	fi

# Création de l'ISO bootable
$(ISO): $(OUTPUT_BIN) multiboot-check
	mkdir -p isodir/boot/grub
	cp $(OUTPUT_BIN) isodir/boot/
	cp grub.cfg isodir/boot/grub/
	grub-mkrescue -o $(ISO) isodir

# Lancement dans QEMU pour tests
run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)

# Nettoyage
clean:
	rm -rf *.o $(OUTPUT_BIN) $(ISO) isodir

.PHONY: all clean run multiboot-check

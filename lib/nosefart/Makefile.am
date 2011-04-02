noinst_LTLIBRARIES=libnosefart.la

libnosefart_la_SOURCES=src/log.c \
                       src/memguard.c \
                       src/cpu/nes6502/dis6502.c \
                       src/cpu/nes6502/nes6502.c \
                       src/machine/nsf.c \
                       src/sndhrdw/fds_snd.c \
                       src/sndhrdw/fmopl.c \
                       src/sndhrdw/mmc5_snd.c \
                       src/sndhrdw/nes_apu.c \
                       src/sndhrdw/vrc7_snd.c \
                       src/sndhrdw/vrcvisnd.c

libnosefart_la_LDFLAGS=-module -avoid-version
libnosefart_la_CFLAGS=-DNSF_PLAYER

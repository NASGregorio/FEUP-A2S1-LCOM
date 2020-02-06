PROG=proj

SRCS = $(wildcard *.c)
INCS = ./include

# __LCOM_OPTIMIZED__ disables most of the LCF's overhead (and functionality!)
CPPFLAGS += -pedantic -D __LCOM_OPTIMIZED__ -I$(INCS)

DPADD += ${LIBLCF}
LDADD += -llcf -lm

.include <minix.lcom.mk>

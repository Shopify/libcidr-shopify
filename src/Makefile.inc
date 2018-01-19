# Shared stuff among various make invocations

# Library naming and versioning
.include "${_DIRSRC}/../Makefile.inc"

# Basic cc flags (turn on lots of seatbelts)
C_WARNS = -Wall -Wstrict-prototypes -Wredundant-decls \
		-Wmissing-prototypes -Wpointer-arith -Winline \
		-Wcast-align -Wcast-qual -Wchar-subscripts \
		-Winline -Wnested-externs -Wmissing-declarations \
		-Wsign-compare \
		-pedantic \
		#-Werror
_CFLAGS := -g -O2 -pipe ${C_WARNS} -I../include -Iinclude ${C_FLAGS}
.ifdef NO_OPTIMIZE
_CFLAGS += -O0
.endif

# We generally want to use C[89]9.  However, some things (like strdup())
# aren't in the standard, though they're essentially universal, and I
# can't be arsed implementing them manually.  Also, some systems get very
# strict about limiting with C99 (Fedora Core, for instance, doesn't
# expose getopt() in unistd.h with -std=c[89]9).  As a stopgap, set
# -std=c99, but override it by requesting X/Open and SUS extensions
# (which includes POSIX 200112).  We could use -std=gnu99 here too, as
# the GNU extensions include what we care about, but that probably drifts
# us too far in the direction of gcc-centricism.
_CFLAGS += -std=c99 -D_XOPEN_SOURCE=600

# Roll 'em all up in a place easy to override
CFLAGS += ${_CFLAGS}

O_FILES = ${C_FILES:%.c=%.o}
SO_FILES = ${C_FILES:%.c=%.So}

CC ?= cc
LD ?= ld
MV ?= mv
LN ?= ln
RM ?= rm
LORDER ?= ../tools/lorder
TSORT ?= tsort

def: all

${SHLIB_NAME}: ${SO_FILES}
	@echo Linking ${SHLIB_NAME}...
	${CC} -shared -Wl,-x -o ${@} -Wl,-install_name,${SHLIB_NAME} \
			`${LORDER} ${SO_FILES} | ${TSORT}`
	${LN} -sf ${SHLIB_NAME} ${SHLIB_LINK}

clean allclean:
	${RM} -f ${CLEAN_FILES}


# bmake needs this to recognize the transform rule
.SUFFIXES: ${.SUFFIXES} .So

.c.o:
	${CC} -c ${CFLAGS} -o ${@} ${<}

.c.So:
	${CC} -fpic -DPIC -c ${CFLAGS} -o ${@} ${<}
	@${LD} -o ${@}.tmp -r ${@}
	@${MV} -f ${@}.tmp ${@}


# This is used for test/examples
.ifdef PROGNAME
# These paths wind up relative to src/{test,examples}/*
CFLAGS += -I../../../include
L_FLAGS += -g -pipe -L../..
LCIDR = ../../libcidr.so

CLEAN_FILES = ${O_FILES} ${PROGNAME} *core .depend

all build: ${PROGNAME}

${PROGNAME}: ${O_FILES} ${LCIDR}
	${CC} ${L_FLAGS} -o ${@} ${O_FILES} -lcidr

${LCIDR}:
	(cd ../.. && make)
.endif

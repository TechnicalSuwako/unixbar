UNAME_S != uname -s
UNAME_M != uname -m
OS = ${UNAME_S}
ARCH = ${UNAME_M}

.if ${UNAME_S} == "OpenBSD"
OS = openbsd
.elif ${UNAME_S} == "NetBSD"
OS = netbsd
.elif ${UNAME_S} == "Linux"
OS = linux
.endif

.if ${UNAME_M} == "x86_64"
ARCH = amd64
.endif

NAME != cat main.c | grep "const char \*sofname" | awk '{print $$5}' |\
	sed "s/\"//g" | sed "s/;//"
VERSION != cat main.c | grep "const char \*version" | awk '{print $$5}' |\
	sed "s/\"//g" | sed "s/;//"

PREFIX = /usr/local
.if ${OS} == "linux"
PREFIX = /usr
.endif

CC = cc
FILES = main.c src/common.c src/battery.c src/time.c src/volume.c

CFLAGS = -Wall -Wextra -I/usr/include -L/usr/lib
.if ${OS} == "netbsd"
CFLAGS += -I/usr/pkg/include -L/usr/pkg/lib -I/usr/X11R7/include -L/usr/X11R7/lib
.elif ${OS} == "openbsd"
CFLAGS += -I/usr/X11R6/include -I/usr/X11R6/include/freetype2 -L/usr/X11R6/lib
.endif

LDFLAGS = -lc -lX11 -lXft
SLIBS = -lxcb -lfontconfig -lz -lexpat -lfreetype -lXrender -lXau -lXdmcp

all:
	${CC} -O3 ${CFLAGS} -o ${NAME} ${FILES} -static ${LDFLAGS} ${SLIBS}
	strip ${NAME}

debug:
	${CC} -g ${CFLAGS} -o ${NAME} ${FILES} ${LDFLAGS}

clean:
	rm -f ${NAME}

dist:
	mkdir -p ${NAME}-${VERSION} release/src
	cp -R LICENSE.txt Makefile README.md CHANGELOG.md\
		main.c src ${NAME}-${VERSION}
	tar zcfv release/src/${NAME}-${VERSION}.tar.gz ${NAME}-${VERSION}
	rm -rf ${NAME}-${VERSION}

release:
	mkdir -p release/bin/${VERSION}/${OS}/${ARCH}
	${CC} -O3 ${CFLAGS} -o release/bin/${VERSION}/${OS}/${ARCH}/${NAME} ${FILES}\
		-static ${LDFLAGS} ${SLIBS}
	strip release/bin/${VERSION}/${OS}/${ARCH}/${NAME}

install:
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f ${NAME} ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/${NAME}

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/${NAME}

.PHONY: all debug clean dist release install uninstall

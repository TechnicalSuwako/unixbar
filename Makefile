UNAME_S != uname -s
UNAME_M != uname -m
OS = ${UNAME_S}
ARCH = ${UNAME_M}

.if ${UNAME_S} == "OpenBSD"
OS = openbsd
.elif ${UNAME_S} == "NetBSD"
OS = netbsd
.elif ${UNAME_S} == "FreeBSD"
OS = freebsd
.elif ${UNAME_S} == "Dragonfly"
OS = dragonfly
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

MANPREFIX = ${PREFIX}/share/man
.if ${OS} == "openbsd"
MANPREFIX = ${PREFIX}/man
.endif

CNFPREFIX = /etc
.if ${OS} == "freebsd" || ${OS} == "netbsd" || ${OS} == "dragonfly"
CNFPREFIX = ${PREFIX}/etc
.endif

CC = cc
FILES = main.c src/common.c src/battery.c src/time.c src/volume.c

CFLAGS = -Wall -Wextra -O3 -I/usr/include -L/usr/lib
.if ${OS} == "netbsd"
CFLAGS += -I/usr/pkg/include -L/usr/pkg/lib -I/usr/X11R7/include -L/usr/X11R7/lib
.elif ${OS} == "openbsd"
CFLAGS += -I/usr/X11R6/include -L/usr/X11R6/lib
.elif ${OS} == "freebsd"
CFLAGS += -I/usr/local/include -L/usr/local/lib
.endif

LDFLAGS = -lc -lX11

all:
	${CC} ${CFLAGS} -o ${NAME} ${FILES} ${LDFLAGS}
	strip ${NAME}

clean:
	rm -f ${NAME}

dist:
	mkdir -p ${NAME}-${VERSION} release/src
	cp -R LICENSE.txt Makefile README.md CHANGELOG.md\
		${NAME}.conf ${NAME}.1 ${NAME}.conf.5 main.c src ${NAME}-${VERSION}
	tar zcfv release/src/${NAME}-${VERSION}.tar.gz ${NAME}-${VERSION}
	rm -rf ${NAME}-${VERSION}

man:
	mkdir -p release/man/${VERSION}
	sed "s/VERSION/${VERSION}/g" < ${NAME}.1 > release/man/${VERSION}/${NAME}.1
	sed "s/VERSION/${VERSION}/g" < ${NAME}.conf.5 > release/man/${VERSION}/${NAME}.conf.5

release:
	mkdir -p release/bin/${VERSION}/${OS}/${ARCH}
	${CC} ${CFLAGS} -o release/bin/${VERSION}/${OS}/${ARCH}/${NAME} ${FILES}\
		-static ${LDFLAGS}
	strip release/bin/${VERSION}/${OS}/${ARCH}/${NAME}

install:
	mkdir -p ${DESTDIR}${PREFIX}/bin ${DESTDIR}${MANPREFIX}/man1\
		${DESTDIR}${MANPREFIX}/man5
	cp -f ${NAME} ${DESTDIR}${PREFIX}/bin
	cp -f ${NAME}.conf ${DESTDIR}${CNFPREFIX}
	chmod 755 ${DESTDIR}${PREFIX}/bin/${NAME}
	sed "s/VERSION/${VERSION}/g" < ${NAME}.1 > ${DESTDIR}${MANPREFIX}/man1/${NAME}.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/${NAME}.1
	sed "s/VERSION/${VERSION}/g" < ${NAME}.conf.5 >\
		${DESTDIR}${MANPREFIX}/man5/${NAME}.conf.5
	chmod 644 ${DESTDIR}${MANPREFIX}/man5/${NAME}.conf.5

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/${NAME}
	rm -rf ${DESTDIR}${PREFIX}/man/man1/${NAME}.1
	rm -rf ${DESTDIR}${PREFIX}/man/man5/${NAME}.conf.5

.PHONY: all clean dist man release install uninstall

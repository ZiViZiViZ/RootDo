CFLAGS = -Wall -Wextra -Werror -Wl,-z,now
CFLAGS_RELEASE = ${CFLAGS} -O2 -s -D_FORTIFY_SOURCE=2
CFLAGS_DEBUG = ${CFLAGS} -O0 -g -fsanitize=undefined
LIBS = -lbsd -lcrypt
CC = gcc

all: rdo.c
	${CC} ${CFLAGS_RELEASE} rdo.c -o rdo ${LIBS}

debug: rdo.c
	${CC} ${CFLAGS_DEBUG} rdo.c -o rdo ${LIBS}

install: rdo
	cp rdo /usr/bin/rdo
	chown root:root /usr/bin/rdo
	chmod 755 /usr/bin/rdo
	chmod u+s /usr/bin/rdo
	cp rdo_sample.conf /etc/rdo.conf
	chmod 600 /etc/rdo.conf

uninstall:
	rm /usr/bin/rdo
	rm /etc/rdo.conf

clean:
	rm rdo

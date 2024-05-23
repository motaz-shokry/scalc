CC = cc
LIBS = -lglfw -lGL -lleif -lm -lclipboard
SRC = *.c
BIN = scalc

all: build

build:
	@if [! -d ./dependencis/leif/lib ]; then \
		$(MAKE) -C ./dependencis/leif/ && $(MAKE) -C ./dependencis/leif/ install; \
	fi
	${CC} -o ${BIN} ${SRC} ${LIBS}

clean:
	$(MAKE) -C ./dependencis/leif/ clean
	rm -f ${BIN}

install:
	cp ${BIN} /usr/bin/
	cp ./scalc.desktop /usr/share/applications/

uninstall:
	rm -f /usr/bin/${BIN}
	rm -f /usr/share/applications/scalc.desktop

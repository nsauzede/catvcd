TARGET=catvcd
TARGET+=catvcd2

CFLAGS=-Wall -g -O0
CXXFLAGS=-Wall -Werror -Wextra -pedantic -std=c++11 -g -O0
INSTALL_PREFIX=	$(USR)/local/bin
CP=	cp -f

all: $(TARGET)

install: all
	$(CP) $(TARGET) $(INSTALL_PREFIX)

clean:
	$(RM) $(TARGET)

check: catvcd2
	./catvcd2 cpu86.vcd

clobber: clean

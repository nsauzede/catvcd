TARGET= catvcd
CFLAGS=	-Wall -g -O0
INSTALL_PREFIX=	$(USR)/local/bin
CP=	cp -f

all:    $(TARGET)

clean:
	$(RM) $(TARGET)

install:	all
	$(CP) $(TARGET) $(INSTALL_PREFIX)

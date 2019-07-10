# installation directory
########################
PREFIX	=	/opt/local

# install prog
##############
INSTALL	=	/usr/sbin/install

# file ownership/modes
######################
OWNER	=	root
GROUP	=	root

# BIN_MODE =	4711	# `uname -r` < 2.6
BIN_MODE=	0755	# `uname -r` >= 2.6 or non-root 2.5.1 install
MAN_MODE=	0444

# manual section/directory (lower case)
#######################################
# MANSEC	=	n
MANSEC	=	1

# compiler flags
################
CPPFLAGS =	-DSOL26
# CFLAGS	=	-g
LDFLAGS	=	$(CFLAGS)


# probably no need to change anything below here
################################################

HEADERS	=	get_pinfo.h
COMMON_OBJS =	get_pinfo.o

BIGMANSEC =	`echo ${MANSEC} | tr a-z A-Z`
.SUFFIXES:	.man .$(MANSEC)

.man.$(MANSEC):
	sed -e "s/@MANSEC@/$(MANSEC)/g" -e "s/@BIGMANSEC@/$(BIGMANSEC)/g" $*.man > $*.$(MANSEC)

all:		killcmd pidof killcmd.$(MANSEC) pidof.$(MANSEC)

killcmd:	killcmd.o $(COMMON_OBJS) $(HEADERS)
		$(CC) $(LDFLAGS) -o $@ $@.o $(COMMON_OBJS)

pidof:		pidof.o $(COMMON_OBJS) $(HEADERS)
		$(CC) $(LDFLAGS) -o $@ $@.o $(COMMON_OBJS)

install:	all
	$(INSTALL) -f $(PREFIX)/bin -s -m $(BIN_MODE) -u $(OWNER) -g $(GROUP) killcmd
	$(INSTALL) -f $(PREFIX)/bin -s -m $(BIN_MODE) -u $(OWNER) -g $(GROUP) pidof
	$(INSTALL) -f $(PREFIX)/man/man$(MANSEC) -s -m $(MAN_MODE) -u $(OWNER) -g $(GROUP) killcmd.$(MANSEC)
	$(INSTALL) -f $(PREFIX)/man/man$(MANSEC) -s -m $(MAN_MODE) -u $(OWNER) -g $(GROUP) pidof.$(MANSEC)

clean:
	rm -f *.o killcmd pidof core *.$(MANSEC)

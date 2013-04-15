COMMON_DIR=.
COMMON_SRCS=$(COMMON_DIR)/servicesfactory.c $(COMMON_DIR)/clientadapter.c $(COMMON_DIR)/service.c $(COMMON_DIR)/connector.c

SERVER_DIR=.
SERVER_TARGET=server
SERVER_SLIB=lib$(SERVER_TARGET).a
SERVER_DLIB=lib$(SERVER_TARGET).so
SERVER_SRCS=server.c $(COMMON_SRCS)

CLIENT_DIR=.
CLIENT_TARGET=client
CLIENT_SLIB=lib$(CLIENT_TARGET).a
CLIENT_DLIB=lib$(CLIENT_TARGET).so
CLIENT_SRCS= client.c $(COMMON_SRCS)

MPILOTE_DIR=.
MPILOTE_TARGET=mpilote
MPILOTE_SLIB=lib$(MPILOTE_TARGET).a
MPILOTE_DLIB=lib$(MPILOTE_TARGET).so
MPILOTE_SRCS=server.c client.c $(COMMON_SRCS)

NETWORKLIBPATH?=../..
INCLUDEDIR?=$(NETWORKLIBPATH)/include
OBJDIR?=../obj
TARGETDIR=$(NETWORKLIBPATH)/$(ARCH)lib

CFLAGS+=

SERVER_SLIB:=$(OBJDIR)/$(SERVER_SLIB)
SERVER_DLIB:=$(TARGETDIR)/$(SERVER_DLIB)
SERVER_OBJS:=$(addprefix $(OBJDIR)/, $(patsubst  %.c,%.o, $(SERVER_SRCS)))

CLIENT_SLIB:=$(OBJDIR)/$(CLIENT_SLIB)
CLIENT_DLIB:=$(TARGETDIR)/$(CLIENT_DLIB)
CLIENT_OBJS:=$(addprefix $(OBJDIR)/, $(patsubst  %.c,%.o, $(CLIENT_SRCS)))

MPILOTE_SLIB:=$(OBJDIR)/$(MPILOTE_SLIB)
MPILOTE_DLIB:=$(TARGETDIR)/$(MPILOTE_DLIB)
MPILOTE_OBJS:=$(addprefix $(OBJDIR)/, $(patsubst  %.c,%.o, $(MPILOTE_SRCS)))

all: server client

$(OBJDIR) $(TARGETDIR):
	mkdir $@

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

server: $(SERVER_DLIB) $(SERVER_SLIB)
	echo $@

client: $(CLIENT_DLIB) $(CLIENT_SLIB)

mpilote: $(MPILOTE_DLIB) $(MPILOTE_SLIB)

$(SERVER_OBJS) $(CLIENT_OBJS) $(MPILOTE_OBJS) : |$(OBJDIR) $(TARGETDIR)

$(SERVER_SLIB): $(SERVER_OBJS)
	$(AR) rcs $@ $^

$(SERVER_DLIB): CFLAGS+=-fPIC -fno-stack-protector
$(SERVER_DLIB): $(SERVER_OBJS)
	$(LD) -shared -fPIC -o $@ $^

$(CLIENT_SLIB): $(CLIENT_OBJS)
	$(AR) rcs $@ $^

$(CLIENT_DLIB): CFLAGS+=-fPIC -fno-stack-protector
$(CLIENT_DLIB): $(CLIENT_OBJS)
	$(LD) -shared -fPIC -o $@ $^

$(MPILOTE_SLIB): $(MPILOTE_OBJS)
	$(AR) rcs $@ $^

$(MPILOTE_DLIB): CFLAGS+=-fPIC -fno-stack-protector
$(MPILOTE_DLIB): $(MPILOTE_OBJS)
	$(LD) -shared -fPIC -o $@ $^

clean: clean_server clean_client clean_objs
	$(RM) -r $(NETWORKLIBPATH)/lib
	$(RM) -r $(OBJDIR)

clean_objs:
	$(RM) $(CLIENT_OBJS) $(SERVER_OBJS) $(MPILOTE_OBJS)

clean_client:
	$(RM) $(CLIENT_DLIB) $(CLIENT_SLIB)

clean_server:
	$(RM) $(SERVER_DLIB) $(SERVER_SLIB)

clean_mpilote:
	$(RM) $(MPILOTE_DLIB) $(MPILOTE_SLIB)

.PHONY: clean clean_client clean_server server client

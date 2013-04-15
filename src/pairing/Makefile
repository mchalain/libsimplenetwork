TARGET=pairing
SRCS=main.c

CTARGET=cpairing
CSRCS=client_main.c

OBJDIR?=../obj
INCLUDEDIR?=$(NETWORKLIBPATH)/include
PREFIX?=/usr
SHAREDIR?=$(PREFIX)/share/$(TARGET)
CFLAGS+=-DSHAREDIR=\"$(SHAREDIR)\"
LDFLAGS+=

SERVICE_DIR?=$(NETWORKLIBPATH)/lib
SERVICE_TARGET=pairing
SERVICE_SLIB=$(SERVICE_DIR)/lib$(SERVICE_TARGET).a
SERVICE_DLIB=$(SERVICE_DIR)/lib$(SERVICE_TARGET).so
SERVICE_SRCS= service_pairing.c

SERVER_DIR?=$(NETWORKLIBPATH)/lib
SERVER_TARGET=server
SERVER_SLIB=$(SERVER_DIR)/lib$(SERVER_TARGET).a
SERVER_DLIB=$(SERVER_DIR)/lib$(SERVER_TARGET).so

CLIENT_DIR?=$(NETWORKLIBPATH)/lib
CLIENT_TARGET=client
CLIENT_SLIB=$(CLIENT_DIR)/lib$(CLIENT_TARGET).a
CLIENT_DLIB=$(CLIENT_DIR)/lib$(CLIENT_TARGET).so

all: $(TARGETDIR)/$(TARGET) $(TARGETDIR)/$(CTARGET)

OBJS:=$(addprefix $(OBJDIR)/, $(patsubst  %.c,%.o, $(SRCS)))
COBJS:=$(addprefix $(OBJDIR)/, $(patsubst  %.c,%.o, $(CSRCS)))
SERVICE_OBJS:=$(addprefix $(OBJDIR)/, $(patsubst  %.c,%.o, $(SERVICE_SRCS)))

$(OBJS): |$(OBJDIR) $(TARGETDIR)

$(OBJDIR) $(TARGETDIR):
	mkdir $@

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(SERVICE_SLIB): $(SERVICE_OBJS)
	$(AR) rcs $@ $^

$(SERVICE_DLIB): $(SERVICE_OBJS)
	$(LD) -shared -fPIC -o $@ $^

$(TARGETDIR)/$(CTARGET): $(COBJS) $(SERVICE_DLIB) $(CLIENT_DLIB)
	$(CC) $^ -o $@ -L$(CLIENT_DIR) -l$(CLIENT_TARGET)  -L$(SERVICE_DIR) -l$(SERVICE_TARGET) $(LDFLAGS)

$(TARGETDIR)/$(TARGET): $(OBJS) $(SERVICE_DLIB) $(SERVER_DLIB)
	$(CC) $^ -o $@ -L$(SERVER_DIR) -l$(SERVER_TARGET)  -L$(SERVICE_DIR) -l$(SERVICE_TARGET) $(LDFLAGS)

static: $(OBJS) $(SERVICE_SLIB) $(SERVER_DIR)/$(SERVER_SLIB)
	$(CC) $^ -o $(TARGET)

clean: clean_objs clean_target clean_service

clean_objs:
	$(RM) $(OBJS) $(SERVICE_OBJS)

clean_target:
	$(RM)  $(TARGET)

clean_service:
	$(RM) $(SERVICE_DLIB) $(SERVICE_SLIB)

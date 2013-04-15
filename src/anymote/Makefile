OBJDIR?=../obj
INCLUDEDIR?=$(NETWORKLIBPATH)/include
PREFIX?=/usr
SHAREDIR?=$(PREFIX)/share/$(TARGET)
# -DDEVICEADAPTER
CFLAGS+=-DSHAREDIR="$(SHAREDIR)" -I$(NETWORKLIBPATH)/../anymotectrl/include
LDFLAGS+=-L$(NETWORKLIBPATH)/../anymotectrl/bin -lanymotectrl
#LDFLAGS= -lefence -lpthread

SERVICE_DIR?=$(NETWORKLIBPATH)/lib
SERVICE_TARGET=anymote
SERVICE_SLIB=$(SERVICE_DIR)/lib$(SERVICE_TARGET).a
SERVICE_DLIB=$(SERVICE_DIR)/lib$(SERVICE_TARGET).so
SERVICE_SRCS= service_anymote.c


SERVER_DIR?=$(NETWORKLIBPATH)/lib
SERVER_TARGET=server
SERVER_SLIB=$(SERVER_DIR)/lib$(SERVER_TARGET).a
SERVER_DLIB=$(SERVER_DIR)/lib$(SERVER_TARGET).so

all: $(SERVICE_DLIB)

OBJS:=$(addprefix $(OBJDIR)/, $(patsubst  %.c,%.o, $(SRCS)))
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

$(TARGETDIR)/$(TARGET): $(OBJS) $(SERVICE_DLIB) $(SERVER_DLIB)
	$(CC) $^ -o $@ -L$(SERVER_DIR) -l$(SERVER_TARGET)  -L$(SERVICE_DIR) -l$(SERVICE_TARGET) $(LDFLAGS)

$(ANYMOTE_TARGET): $(ANYMOTE_OBJS) $(LIBANYMOTE_TARGET)
	$(CC) $^ -o $@

static: $(OBJS) $(SERVICE_SLIB) $(SERVER_SLIB)
	$(CC) $^ -o $(TARGET)

clean: clean_service clean_objs
	$(RM) $(TARGET)

clean_objs:
	$(RM) $(OBJS) $(SERVICE_OBJS)

clean_service:
	$(RM) $(SERVICE_DLIB) $(SERVICE_SLIB)

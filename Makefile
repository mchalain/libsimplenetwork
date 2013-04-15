TARGETS=minihttpd
CP=cp

SERVER=server
CLIENT=client
MPILOTE=mpilote

SUBDIRS= src/lib src/test src/http src/pairing src/anymote

export NETWORKLIB_SERVER=server
export NETWORKLIB_CLIENT=client
export NETWORKLIBPATH=$(PWD)
export TARGETDIR=$(PWD)/bin
export OBJDIR:=$(PWD)/obj
export SERVER_DIR:=$(PWD)/lib
export CLIENT_DIR:=$(PWD)/lib
export SERVICE_DIR:=$(PWD)/lib
export CFLAGS:=-g -Wall -I$(PWD)/include
export LDFLAGS:=-ldl

all:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $(ARG); \
	done

$(SERVER) $(CLIENT) $(MPILOTE):
	$(MAKE) -C src/lib $@

pairing anymote http rtp testserver testclient: $(SERVER) $(CLIENT)
	$(MAKE) -C src/$@ clean_objs
	$(MAKE) -C src/$@

clean: ARG=clean
clean:all
	$(RM) -rf $(OBJDIR) $(TARGETDIR) $(SERVER_DIR) $(SERVICE_DIR) $(CLIENT_DIR)

.PHONY: clean minihttpd testclient testserver

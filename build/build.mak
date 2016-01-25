CC=clang
CXX=clang++
MAKE=make

CFLAGS+=-g -Wall
CXXFLAGS+=-g -Wall

AR=ar cr
RM=-rm -rf

# dirs:=$(shell find . -maxdepth 1 -type d)
# dirs:=$(basename $(patsubst ./%,%,$(dirs)))
# dirs:=$(filter-out $(exclude_dirs),$(dirs))
SUBDIRS:=$(dirs)


# Phony Target
default:$(EXEPATH) $(LIBPATH) $(TARGET) $(LIB) subdirs

$(EXEPATH):
	mkdir $(EXEPATH)
$(LIBPATH):
	mkdir $(LIBPATH)


# First Layer
$(TARGET):$(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
ifdef EXEPATH
	cp $@ $(EXEPATH)
endif

$(LIB):$(OBJS)
	$(AR)  $@  $^
ifdef LIBPATH
	cp $@ $(LIBPATH)
endif

subdirs:$(SUBDIRS)
	for dir in $(SUBDIRS); \
		do $(MAKE) -C $$dir||exit 1; \
	done


# clean
clean:
	for dir in $(SUBDIRS);\
		do $(MAKE) -C $$dir clean||exit 1;\
	done
	$(RM) $(TARGET) $(LIB)  $(OBJS) $(DEPENDS)


# for debug
fake:
	@echo $(OBJS)

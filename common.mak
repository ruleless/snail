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

SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:%.cpp=%.o)
DEPENDS=$(SRCS:%.cpp=%.d)


.PHONY:all fake

# First Layer
all:subdirs $(TARGET) $(LIB)

subdirs:$(SUBDIRS)
	for dir in $(SUBDIRS); \
		do $(MAKE) -C $$dir all||exit 1; \
	done

$(TARGET):$(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)
ifdef $(EXEPATH)
	cp $@ $(EXEPATH)
endif

$(LIB):$(OBJS)
	$(AR)  $@  $^
	cp $@ $(LIBPATH)


# Second Layer for object file deprule
$(OBJS):%.o:%.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

# Second Layer for dependency file deprule
-include $(DEPENDS)

$(DEPENDS):%.d:%.cpp
	set -e; rm -f $@; \
	$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[:]*,\1.o $@:,g' < $@.$$$$ > $@; \
	rm $@.$$$$


# clean
clean:
	for dir in $(SUBDIRS);\
		do $(MAKE) -C $$dir clean||exit 1;\
	done
	$(RM) $(TARGET) $(LIB)  $(OBJS) $(DEPENDS)


# for debug
fake:

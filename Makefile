SYSCONF_LINK = g++
CPPFLAGS     =
LDFLAGS      =
LIBS         = -lm

# 发布版本的编译选项
RELEASE_FLAGS = -O3 -Wall
# 调试版本的编译选项
DEBUG_FLAGS   = -ggdb -g -pg -O0 -Wall -DDEBUG

DESTDIR = ./
TARGET  = main
TARGET_DEBUG = $(TARGET)_debug

OBJECTS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

.PHONY: all clean debug release

all: debug

release: CPPFLAGS += $(RELEASE_FLAGS)
release: $(DESTDIR)$(TARGET)

debug: CPPFLAGS += $(DEBUG_FLAGS)
debug: LDFLAGS += -pg 	# 使用gprof时，链接过程也需要-pg选项
debug: $(DESTDIR)$(TARGET_DEBUG)

$(DESTDIR)$(TARGET): $(OBJECTS)
	$(SYSCONF_LINK) $(LDFLAGS) -o $(DESTDIR)$(TARGET) $(OBJECTS) $(LIBS)

$(DESTDIR)$(TARGET_DEBUG): $(OBJECTS)
	$(SYSCONF_LINK) $(LDFLAGS) -o $(DESTDIR)$(TARGET_DEBUG) $(OBJECTS) $(LIBS)

$(OBJECTS): %.o: %.cpp
	$(SYSCONF_LINK) $(CPPFLAGS) -c $(CFLAGS) $< -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)
	-rm -f $(TARGET_DEBUG)
	-rm -f *.tga

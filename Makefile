SYSCONF_LINK = g++
CPPFLAGS     =
LDFLAGS      =
LIBS         = -lm

# 编译选项
RELEASE_FLAGS = -O3 -Wall
DEBUG_FLAGS   = -ggdb -g -pg -O0 -Wall -DDEBUG

# 链接选项
RELEASE_FLAGS_LD = 
DEBUG_FLAGS_LD = -pg

# 目录和目标设置
DESTDIR = ./
TARGET  = tinyrenderer
TARGET_DEBUG = $(TARGET)_debug
LINEBENCH_TARGET = line_bench
TRIANGLEBENCH_TARGET = triangle_bench
ZBUFBENCH_TARGET = zbuf_bench
ALL_TARGET = $(TARGET) $(TARGET_DEBUG) $(LINEBENCH_TARGET) $(TRIANGLEBENCH_TARGET) $(ZBUFBENCH_TARGET)

# 源文件
MAIN_SRCS = main.cpp tgaimage.cpp model.cpp
LINEBENCH_SRCS = linebench_main.cpp tgaimage.cpp
TRIANGLEBENCH_SRCS = trianglebench_main.cpp tgaimage.cpp
ZBUFBENCH_SRCS = zbufbench_main.cpp tgaimage.cpp

# object文件,等待链接
MAIN_OBJS = $(MAIN_SRCS:.cpp=.o)
LINEBENCH_OBJS = $(LINEBENCH_SRCS:.cpp=.o)
TRIANGLEBENCH_OBJS = $(TRIANGLEBENCH_SRCS:.cpp=.o)
ZBUFBENCH_OBJS = $(ZBUFBENCH_SRCS:.cpp=.o)
OBJECTS = $(MAIN_OBJS) $(LINEBENCH_OBJS) $(TRIANGLEBENCH_OBJS) $(ZBUFBENCH_OBJS)

.PHONY: all clean debug release bench help

all: debug

# 主程序的release版本
release: CPPFLAGS += $(RELEASE_FLAGS)
release: LDFLAGS += $(RELEASE_FLAGS_LD)
release: $(DESTDIR)$(TARGET)

# 主程序的debug版本
debug: CPPFLAGS += $(DEBUG_FLAGS)
debug: LDFLAGS += $(DEBUG_FLAGS_LD)
debug: $(DESTDIR)$(TARGET_DEBUG)

# 线条绘制基准测试
linebench: CPPFLAGS += $(DEBUG_FLAGS)
linebench: LDFLAGS += $(DEBUG_FLAGS_LD)
linebench: $(DESTDIR)$(LINEBENCH_TARGET)

# 三角形绘制基准测试
trianglebench: CPPFLAGS += $(DEBUG_FLAGS)
trianglebench: LDFLAGS += $(DEBUG_FLAGS_LD)
trianglebench: $(DESTDIR)$(TRIANGLEBENCH_TARGET)

# z缓冲基准测试
zbufbench: CPPFLAGS += $(DEBUG_FLAGS)
zbufbench: LDFLAGS += $(DEBUG_FLAGS_LD)
zbufbench: $(DESTDIR)$(ZBUFBENCH_TARGET)

# 编译所有基准测试
bench: linebench trianglebench zbufbench

# 主程序的链接规则
$(DESTDIR)$(TARGET): $(MAIN_OBJS)
	$(SYSCONF_LINK) $(LDFLAGS) -o $@ $^ $(LIBS)

$(DESTDIR)$(TARGET_DEBUG): $(MAIN_OBJS)
	$(SYSCONF_LINK) $(LDFLAGS) -o $@ $^ $(LIBS)

# 基准测试程序的链接规则
$(DESTDIR)$(LINEBENCH_TARGET): $(LINEBENCH_OBJS)
	$(SYSCONF_LINK) $(LDFLAGS) -o $@ $^ $(LIBS)

$(DESTDIR)$(TRIANGLEBENCH_TARGET): $(TRIANGLEBENCH_OBJS)
	$(SYSCONF_LINK) $(LDFLAGS) -o $@ $^ $(LIBS)

$(DESTDIR)$(ZBUFBENCH_TARGET): $(ZBUFBENCH_OBJS)
	$(SYSCONF_LINK) $(LDFLAGS) -o $@ $^ $(LIBS)

# 通用的.o文件编译规则
%.o: %.cpp
	$(SYSCONF_LINK) $(CPPFLAGS) -c $< -o $@

help:
	@echo "TinyRenderer Makefile Help"
	@echo "=========================="
	@echo ""
	@echo "Main Targets:"
	@echo "  make all        - Same as 'make debug'"
	@echo "  make debug      - Build debug version with symbols ($(TARGET_DEBUG))"
	@echo "  make release    - Build release version with optimizations ($(TARGET))"
	@echo ""
	@echo "Benchmark Targets:"
	@echo "  make bench      - Build all benchmark versions"
	@echo ""
	@echo "Other Targets:"
	@echo "  make clean      - Remove all generated files"
	@echo "  make purge      - Remove all related files('make clean' and benchmark results)"
	@echo "  make help       - Show this help message"
	@echo ""

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)
	-rm -f $(TARGET_DEBUG)
	-rm -f $(ALL_TARGET)
	-rm -f *.tga

purge: clean
	-rm -f *.out analysis*.txt

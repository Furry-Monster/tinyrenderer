CXX          = g++
CXXFLAGS     = -std=c++17 -Wall -Wextra
LDFLAGS      =
LIBS         = -lm

# 编译选项
RELEASE_FLAGS = -O3 -march=native -DNDEBUG
DEBUG_FLAGS   = -g3 -ggdb -O0 -DDEBUG -pg
COMMON_FLAGS  = -MMD -MP

# 链接选项
RELEASE_FLAGS_LD = -s
DEBUG_FLAGS_LD = -pg

# 头文件目录
INCLUDES = -I. -Iinclude
BUILD_DIR = build
RELEASE_DIR = $(BUILD_DIR)/release
DEBUG_DIR = $(BUILD_DIR)/debug
BENCH_DIR = $(BUILD_DIR)/bench

# 目标设置
$(shell mkdir -p $(RELEASE_DIR) $(DEBUG_DIR) $(BENCH_DIR))
TARGET  = tinyrenderer
DEBUG_TARGET = $(TARGET)_debug
LINEBENCH_TARGET = line_bench
TRIANGLEBENCH_TARGET = triangle_bench
ZBUFBENCH_TARGET = zbuf_bench
MATRIXBENCH_TARGET = matrix_bench
ALL_TARGET = $(TARGET) $(DEBUG_TARGET) $(LINEBENCH_TARGET) $(TRIANGLEBENCH_TARGET) $(ZBUFBENCH_TARGET) $(MATRIXBENCH_TARGET)

# 源文件
MAIN_SRCS = main.cpp tgaimage.cpp model.cpp renderer.cpp
LINEBENCH_SRCS = linebench_main.cpp tgaimage.cpp
TRIANGLEBENCH_SRCS = trianglebench_main.cpp tgaimage.cpp
ZBUFBENCH_SRCS = zbufbench_main.cpp tgaimage.cpp
MATRIXBENCH_SRCS = matrixbench_main.cpp tgaimage.cpp model.cpp

# 目标文件规则
DEBUG_OBJS = $(MAIN_SRCS:%.cpp=$(DEBUG_DIR)/%.o)
DEBUG_DEPS = $(DEBUG_OBJS:.o=.d)

RELEASE_OBJS = $(MAIN_SRCS:%.cpp=$(RELEASE_DIR)/%.o)
RELEASE_DEPS = $(RELEASE_OBJS:.o=.d)

LINEBENCH_OBJS = $(LINEBENCH_SRCS:%.cpp=$(BENCH_DIR)/%.o)
TRIANGLEBENCH_OBJS = $(TRIANGLEBENCH_SRCS:%.cpp=$(BENCH_DIR)/%.o)
ZBUFBENCH_OBJS = $(ZBUFBENCH_SRCS:%.cpp=$(BENCH_DIR)/%.o)
MATRIXBENCH_OBJS = $(MATRIXBENCH_SRCS:%.cpp=$(BENCH_DIR)/%.o)
BENCH_DEPS = $(LINEBENCH_OBJS:.o=.d) $(TRIANGLEBENCH_OBJS:.o=.d) $(ZBUFBENCH_OBJS:.o=.d) $(MATRIXBENCH_OBJS:.o=.d)

# 包含所有生成的依赖文件
-include $(DEBUG_DEPS) $(RELEASE_DEPS) $(BENCH_DEPS)

.PHONY: all clean debug release bench help

all: debug

# 主程序的release版本
release: CXXFLAGS += $(RELEASE_FLAGS)
release: LDFLAGS += $(RELEASE_FLAGS_LD)
release: $(RELEASE_DIR)/$(TARGET)

# 主程序的debug版本
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: LDFLAGS += $(DEBUG_FLAGS_LD)
debug: $(DEBUG_DIR)/$(DEBUG_TARGET)

# 线条绘制基准测试
linebench: CXXFLAGS += $(DEBUG_FLAGS)
linebench: LDFLAGS += $(DEBUG_FLAGS_LD)
linebench: $(BENCH_DIR)/$(LINEBENCH_TARGET)

# 三角形绘制基准测试
trianglebench: CXXFLAGS += $(DEBUG_FLAGS)
trianglebench: LDFLAGS += $(DEBUG_FLAGS_LD)
trianglebench: $(BENCH_DIR)/$(TRIANGLEBENCH_TARGET)

# z缓冲基准测试
zbufbench: CXXFLAGS += $(DEBUG_FLAGS)
zbufbench: LDFLAGS += $(DEBUG_FLAGS_LD)
zbufbench: $(BENCH_DIR)/$(ZBUFBENCH_TARGET)

# 矩阵计算基准测试
matrixbench: CXXFLAGS += $(DEBUG_FLAGS)
matrixbench: LDFLAGS += $(DEBUG_FLAGS_LD)
matrixbench: $(BENCH_DIR)/$(MATRIXBENCH_TARGET)

# 编译所有基准测试
bench: linebench trianglebench zbufbench matrixbench

# 主程序的链接规则
$(RELEASE_DIR)/$(TARGET): $(RELEASE_OBJS)
	@echo "Linking (Release): $<"
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

$(DEBUG_DIR)/$(DEBUG_TARGET): $(DEBUG_OBJS)
	@echo "Linking (Debug): $<"
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

# 基准测试程序的链接规则
$(BENCH_DIR)/$(LINEBENCH_TARGET): $(LINEBENCH_OBJS)
	@echo "Linking (Bench): $<"
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BENCH_DIR)/$(TRIANGLEBENCH_TARGET): $(TRIANGLEBENCH_OBJS)
	@echo "Linking (Bench): $<"
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BENCH_DIR)/$(ZBUFBENCH_TARGET): $(ZBUFBENCH_OBJS)
	@echo "Linking (Bench): $<"
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

$(BENCH_DIR)/$(MATRIXBENCH_TARGET): $(MATRIXBENCH_OBJS)
	@echo "Linking (Bench): $<"
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

# 编译规则
$(DEBUG_DIR)/%.o: %.cpp
	@echo "Compiling (Debug): $<"
	@$(CXX) $(CXXFLAGS) $(COMMON_FLAGS) $(DEBUG_FLAGS) $(INCLUDES) -c $< -o $@

$(RELEASE_DIR)/%.o: %.cpp
	@echo "Compiling (Release): $<"
	@$(CXX) $(CXXFLAGS) $(COMMON_FLAGS) $(RELEASE_FLAGS) $(INCLUDES) -c $< -o $@

$(BENCH_DIR)/%.o: %.cpp
	@echo "Compiling (Bench): $<"
	@$(CXX) $(CXXFLAGS) $(COMMON_FLAGS) $(DEBUG_FLAGS) $(INCLUDES) -c $< -o $@

help:
	@echo "TinyRenderer Makefile Help"
	@echo "=========================="
	@echo ""
	@echo "Main Targets:"
	@echo "  make all        - Same as 'make debug'"
	@echo "  make debug      - Build debug version with symbols ($(DEBUG_TARGET))"
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
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR)
	@rm -f *.tga

purge: clean
	@echo "Removing all generated files..."
	@rm -f *.out analysis*.txt

# 变量定义
CC = arm-linux-gcc
CFLAGS = -I/home/gec/jpegDIR/jpeg-9d/build/include -I./include
LDFLAGS = -L/home/gec/jpegDIR/jpeg-9d/build/lib
LIBS = -ljpeg

SRCDIR = src
INCDIR = include
OBJDIR = build
SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRC))

TARGET = $(OBJDIR)/main

# 默认目标
all: $(OBJDIR) $(TARGET)

# 编译目标
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS) $(LIBS)

# 编译每个源文件
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 创建构建目录
$(OBJDIR):
	mkdir -p $(OBJDIR)

# 清理目标
clean:
	rm -rf $(OBJDIR) $(TARGET)
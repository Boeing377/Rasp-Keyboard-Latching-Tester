CC = gcc
CFLAGS = -Wall -I./src
LDFLAGS = -lgpiod -lm

SRCS = main.c ./src/kb_ev_det.c ./src/result_data_processing.c ./src/kb_lat_test.c
OBJS = $(SRCS:.c=.o)
TARGET = keyboard_latency_test

debug: CFLAGS += -DDEBUG -g
debug: all

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)
	rm -f $(OBJS) # 生成目标程序后移除所有的 .o 文件

%.o: %.c ./src/kb_ev_det.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
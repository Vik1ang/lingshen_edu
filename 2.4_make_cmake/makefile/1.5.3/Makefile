.PHONY: clean
CC = gcc
RM = rm
EXE = simple
SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

$(EXE): $(OBJS)
	$(CC) -o $@ $^
%.o: %.c
	$(CC) -o $@ -c $^
clean:
	$(RM) $(EXE) $(OBJS)
src: # 测试make src显示相应的xx.c
	@echo $(SRCS)
objs: # 测试make objs显示对应的xx.o
	@echo $(OBJS)

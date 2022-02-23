ROOTPATH=../../..
XJC=$(ROOTPATH)/xjc/xjc
SRCS=$(wildcard *.js) $(wildcard *.mjs)
TARGET=a.ast
DIFF=diff
EXPECT_TXT=expect.txt

default: test

compile:
	$(XJC) --ast-dump -o $(TARGET) $(SRCS)

run:
	$(XJR) $(TARGET)

test:
	$(XJC) --ast-dump -o $(TARGET) $(SRCS)
	$(DIFF) $(TARGET) $(EXPECT_TXT)

clean:
	$(RM) $(TARGET) gmon.out

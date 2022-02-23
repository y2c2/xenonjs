ROOTPATH=../../..
XJC=$(ROOTPATH)/xjc/xjc
SRCS=$(wildcard *.mjs)
TARGET=a.x
DIFF=diff
ENTRY=f
RESULT_TXT = result.txt
EXPECT_TXT = expect.txt

default: test

compile:
	$(XJC) -o $(TARGET) $(SRCS) --entry $(ENTRY)

run:
	$(XJR) $(TARGET)

test:
	$(XJC) --ret-zero -o $(TARGET) $(SRCS) --entry $(ENTRY) 2> $(RESULT_TXT)
	@dos2unix -q $(RESULT_TXT)
	$(DIFF) $(RESULT_TXT) $(EXPECT_TXT)

clean:
	$(RM) $(TARGET) $(RESULT_TXT) gmon.out

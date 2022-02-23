ROOTPATH=../../..
XJC=$(ROOTPATH)/xjc/xjc
SRCS=$(wildcard *.js) $(wildcard *.mjs)
TARGET=a.x
DIFF=diff
RESULT_TXT = result.txt
EXPECT_TXT = expect.txt

default: test

compile:
	$(XJC) -o $(TARGET) $(SRCS)

run:
	$(XJR) $(TARGET)

test:
	$(XJC) --ret-zero -o $(TARGET) $(SRCS) 2> $(RESULT_TXT)
	@dos2unix -q $(RESULT_TXT)
	$(DIFF) $(RESULT_TXT) $(EXPECT_TXT)

clean:
	$(RM) $(TARGET) $(RESULT_TXT) gmon.out

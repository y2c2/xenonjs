ROOTPATH=../../..
XJC=$(ROOTPATH)/xjc/xjc
XJR=$(ROOTPATH)/xjr/xjr
SRCS=$(wildcard *.js) $(wildcard *.mjs)
ENTRY=m0
TARGET=a.x
DIFF=diff
RESULT_TXT=result.txt
EXPECT_TXT=expect.txt

default:
	$(XJC) -o $(TARGET) $(SRCS) --entry $(ENTRY)

run:
	$(XJR) $(TARGET)

test:
	$(XJR) $(TARGET) > $(RESULT_TXT)
	@dos2unix -q $(RESULT_TXT)
	$(DIFF) $(RESULT_TXT) $(EXPECT_TXT)

clean:
	$(RM) $(TARGET) $(RESULT_TXT) gmon.out

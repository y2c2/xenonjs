ROOTPATH=../../..
XJC=$(ROOTPATH)/xjc/xjc
XJR=$(ROOTPATH)/xjr/xjr
XJDB=$(ROOTPATH)/xjdb/xjdb
SRCS=$(wildcard *.js)
TARGET=a.x
DIFF=diff
RESULT_TXT = result.txt
EXPECT_TXT = expect.txt

default:
	$(XJC) -o $(TARGET) $(SRCS)

explain:
	$(XJC) -S -o - $(SRCS)

run:
	$(XJR) $(TARGET)

debug_server:
	$(XJR) --debug-server 127.0.0.1:19223 $(TARGET)

debug_client:
	$(XJDB) --remote-target 127.0.0.1:19223 $(TARGET)

test:
	$(XJR) $(TARGET) > $(RESULT_TXT)
	@dos2unix -q $(RESULT_TXT)
	$(DIFF) $(RESULT_TXT) $(EXPECT_TXT)

clean:
	$(RM) $(TARGET) $(RESULT_TXT) gmon.out

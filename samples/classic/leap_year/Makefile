ROOTPATH=../../..
XJC=$(ROOTPATH)/xjc/xjc
XJR=$(ROOTPATH)/xjr/xjr
SRCS=$(wildcard *.js)
TARGET=a.x
DIFF=diff
RESULT_TXT = result.txt
EXPECT_TXT = expect.txt
RESULT_AST = result.ast
EXPECT_AST = expect.ast

default:
	$(XJC) -g -o $(TARGET) $(SRCS)

explain:
	$(XJC) -S -g -o - $(SRCS)

run:
	$(XJR) $(TARGET)

test:
	$(XJC) --ast-dump -o $(RESULT_AST) $(SRCS)
	@dos2unix -q $(RESULT_AST)
	$(DIFF) $(RESULT_AST) $(EXPECT_AST)
	$(XJR) $(TARGET) > $(RESULT_TXT)
	@dos2unix -q $(RESULT_TXT)
	$(DIFF) $(RESULT_TXT) $(EXPECT_TXT)

prof:
	gprof $(XJR) gmon.out

clean:
	$(RM) $(TARGET) $(RESULT_TXT) $(RESULT_AST) gmon.out

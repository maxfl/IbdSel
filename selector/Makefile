.PHONY: all clean stage1 stage2

all: stage1 stage2

clean:
	find . -regex '.*\.\(so\|pcm\|d\)' | xargs rm -f

common := $(shell find common external SelectorFramework/core -type f)

stage1: stage1/stage1_main_cc.so

stage2: stage2/stage2_main_cc.so

%_cc.so: %.cc $(common)
	root -l -b -q 'cling/Build.C("$<")'

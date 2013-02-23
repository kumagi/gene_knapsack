CXX=g++
OPTS=-O4 -fexceptions -g

WARNS= -W -Wall -Wextra -Wformat=2 -Wstrict-aliasing=4 -Wcast-qual -Wcast-align \
	-Wwrite-strings -Wfloat-equal -Wpointer-arith -Wswitch-enum
NOTIFY=&& notify-send Test success! -i ~/themes/ok_icon.png || notify-send Test failed... -i ~/themes/ng_icon.png
SRCS=$(HEADS) $(BODYS)

target:generator

generator:generator.cpp
	$(CXX) $^ -o $@ $(WARNS)

clean:
	rm *.o
	rm knapsack
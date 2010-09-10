CXX=g++
OPTS=-O4 -fexceptions -g


#TEST_LD= -lpthread $(LD)
#GTEST_INC= -I$(GTEST_DIR)/include -I$(GTEST_DIR)
#GTEST_DIR=/opt/google/gtest-1.5.0
WARNS= -W -Wall -Wextra -Wformat=2 -Wstrict-aliasing=4 -Wcast-qual -Wcast-align \
	-Wwrite-strings -Wfloat-equal -Wpointer-arith -Wswitch-enum
NOTIFY=&& notify-send Test success! -i ~/themes/ok_icon.png || notify-send Test failed... -i ~/themes/ng_icon.png
SRCS=$(HEADS) $(BODYS)

target:knapsack

knapsack: knapsack.o
	$(CXX) $^ -o $@ $(WARNS) $(OPTS)

knapsack.o: knapsack.cc knapsack.hpp
	$(CC) -c knapsack.cc -o $@ $(WARNS) $(OPTS)

clean:
	rm *.o
	rm knapsack
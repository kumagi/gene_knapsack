#ifndef KNAPSACK_HPP
#define KNAPSACK_HPP

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <algorithm>

struct item{
	int value_;
	int weight_;
	item(int value, int weight):value_(value),weight_(weight){}
	int weight(void)const{return weight_;}
	int value(void)const{return value_;}
	double get_dencity(void)const
	{return static_cast<double>(value_) / weight_;}
	bool operator<(const item& rhs)const
	{return get_dencity() < rhs.get_dencity();}
	bool operator==(const item& rhs)const
	{return weight_ == rhs.weight_ && value_ == rhs.value_;}
	
	void dump()const{
		std::cout << "v:" << value_ << "\tw:" << weight_ << std::endl;
	}
};

class rand_bit{
	int cnt;
	int random;
public:
	rand_bit():cnt(32),random(rand()){}
	bool get(void){
		if(cnt){--cnt;bool ans=(random&1)==1;random>>=1;return ans;}
		else {cnt = 32; random=rand(); return get();}
	}
};


#endif

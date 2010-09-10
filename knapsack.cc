
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <vector>

#include "knapsack.hpp"


#ifndef NDEBUG
#define DEBUG(...) fprintf(stderr,__VA_ARGS__)
#define IN_DEBUG(x) x
#else
#define DEBUG(...)
#define IN_DEBUG(x)
#endif

template<typename Derived>
class singleton {
public:
	static Derived& instance() { static Derived the_inst; return the_inst; }
protected:
	singleton(){}
	~singleton(){}
private:
	singleton(const singleton<Derived>&);
	singleton& operator=(const singleton<Derived>&);
};

struct settings : public singleton<settings>{
	int random_seed;
	std::string filename;
	int filesize;
	int idv_number;
	int mutation_rate;
};


struct problem : public singleton<problem>{
	int sack_size;
	int item_quantum;
	std::vector<item> itemset;
	void dump()const{
		std::cout << "sack_size: " << sack_size << std::endl;
		std::for_each(itemset.begin(),itemset.end(),std::mem_fun_ref(&item::dump));
	}
};

class gene{
private:
	std::vector<bool> gen;
public:
	gene(){}
	void dump()const{
		std::cout << "[";
		for(size_t i = 0;i<gen.size() ; i++){
			std::cout << (gen[i] ? "1" : "0");
		}
		std::cout << "|" << eval() << "] ";
	}
	std::vector<bool> get_gene(void)const{return gen;}
	int eval()const{
		const problem& p = problem::instance();
		int ans = 0;
		int weight = 0;
		for(size_t i=0;i<gen.size();i++){
			if(gen[i]){
				ans += p.itemset[i].value();
				weight += p.itemset[i].weight();
			}
		}
		return weight > p.sack_size ? 0 : ans;
	}
	inline bool get(const int i)const{
		return static_cast<bool>(gen[i/8] & (1 << (i%8)));
	}
	gene& operator+=(const bool& t){
		gen.push_back(t);
		return *this;
	}
	inline void reserve(int newlength){
		gen.reserve(newlength/8+1);
	}
	bool operator==(const gene& rhs)const{
		for(size_t i = 0; i<gen.size(); ++i){
			if(gen[i] != rhs.gen[i]) return false;
		}return true;
		if(gen.size() != rhs.gen.size()) return false;
		for(size_t i=0;i<gen.size()/8; i++){
			if(gen[i] != rhs.gen[i]) return false;
		}
		if((gen.size() % 8) == 0) return true;
		return ((gen[gen.size()/8] ^ rhs.gen[gen.size()/8]) &
			(((1 <<(gen.size() & 7)) + 1) - 1)) == 0;
	}
	bool operator<(const gene& rhs)const{
		return eval() < rhs.eval();
	}

	gene operator/(const gene& rhs)const{
		gene child;
		child.reserve(gen.size()*8);
		rand_bit rb;
		for(size_t i=0;i<gen.size();i++){
			child += rand() & 15 ?
				(rb.get() ? gen[i] : rhs.gen[i])
				: !gen[i];
		}
		return gene(child);
	}
};

class generation{
	std::multiset<gene> idvs;
public:
	void random_set(int number, const int len){
		rand_bit rb;
		while(number > 0){
			gene indv;
//			indv.reserve(len);
			for(int i=len; i>0; --i){indv+=rb.get();}
			idvs.insert(indv);
			--number;
		}
	}
	size_t size(void)const{return idvs.size();}
	void insert_indv(const gene& indv){
		idvs.insert(indv);
	}
	void eliminate_poor(int next_size){
		//std::random_shuffle(idvs.begin(),idvs.end());
		assert(next_size >0);
		std::multiset<gene>::iterator it = idvs.begin();
		while(next_size--) it++;
		idvs.erase(it,idvs.end());
	}
	bool one_gene()const{
		for(std::multiset<gene>::const_iterator it = idvs.begin();it!=idvs.end();++it){
			if(!(*idvs.begin() == *it)) return false;
		}
		return true;
	}
	const gene& operator[](const size_t idx)const{
		assert(idx < idvs.size());
		std::multiset<gene>::const_iterator it = idvs.begin();
		for(size_t i=0;i<idx;++i){
			++it;
		}
		return *it;
	}
	void sort(){
		//std::sort(idvs.rbegin(), idvs.rend());
	}
	void dump()const{
		std::for_each(idvs.begin(),idvs.end(),std::mem_fun_ref(&gene::dump));
	}
	class iterator;
	typedef const iterator const_iterator;
	class iterator{
		std::multiset<gene>::iterator it;
	public:
		iterator(std::multiset<gene>::iterator _it):it(_it){};
		iterator& operator++(int){ ++it; return *this;}
		bool operator==(const iterator& rhs)const{return it == rhs.it;};
		bool operator!=(const iterator& rhs)const{return it != rhs.it;};
		const gene& operator*()const{return *it;}
		const gene* operator->()const{return &*it;}
	};
	
	iterator begin(){ return iterator(idvs.begin());}
	iterator end(){ return iterator(idvs.end());}
	const_iterator begin()const{ return iterator(idvs.begin());}
	const_iterator end()const{ return iterator(idvs.end());}
};

	
struct roulette{
	std::vector<int> border;
	const int entries;
	int sum;
	roulette(const generation& world):entries(world.size()),sum(0){
		border.reserve(world.size());
		for(generation::iterator it=world.begin(); it != world.end(); it++){
			int evaled = it->eval();
			evaled += evaled == 0 ? 1 : 0;
			sum += evaled;
			border.push_back(sum);
		}
	}
	int spin(void)const{
		const int entry = rand() % sum;
		return std::distance(border.begin(), std::lower_bound(border.begin(),border.end(),entry));
	}
};


int main(int argc,char** argv){
	settings& s = settings::instance();
	problem& p = problem::instance();
	{ // parse options
		if(argc < 2){fprintf(stderr,"input filename\n");exit(1);}
		s.filename = argv[1];
		printf("filename: %s \n", s.filename.c_str());
		if(argc >= 3){
			s.idv_number = atoi(argv[2]);
		}else{
			s.idv_number = time(NULL);
		}
		if(argc >= 4){
			s.mutation_rate = atoi(argv[3]);
		}else{
			s.mutation_rate = 15;
		}
		if(argc >= 5){
			s.random_seed = atoi(argv[4]);
		}else{
			s.random_seed = time(NULL);
		}
		printf("seed: %d \n", s.random_seed);
	}
	{ // read file
		std::ifstream file(s.filename.c_str());
		
		if(!file.is_open()){
			perror("open");
			fprintf(stderr,"->[%s]\n",s.filename.c_str());
			exit(1);
		}
		std::string buff;
		file >> buff; // knapsize
		p.sack_size = atoi(buff.c_str());
		while(! file.eof()){
			std::string v,w;
			file >> v; // value
			file >> w; // weight
			if(atoi(v.c_str()) == 0)continue; 
			p.itemset.push_back(item(atoi(v.c_str()), atoi(w.c_str())));
		}
		p.item_quantum = p.itemset.size();
		std::sort(p.itemset.rbegin(), p.itemset.rend());
		IN_DEBUG(p.dump());
		DEBUG("item:%d\n",p.itemset.size());
	}
	srand(s.random_seed);
	
	generation world;
	world.random_set(512, p.item_quantum);
	//IN_DEBUG(world.sort());
	//IN_DEBUG(world.dump());
	
	// start genetic algorithm
	{
		const int new_generation = world.size();
		int cnt = 0;
		while(!world.one_gene()){
			const roulette rlt(world);
			for(int i=0; i < new_generation; i++){
				const gene& parent1(world[rlt.spin()]);
				const gene& parent2(world[rlt.spin()]);
				world.insert_indv(parent1 / parent2);
			}
			world.dump();
			world.eliminate_poor(world.size()/2);
			++cnt;
		}
		world[0].dump();
		std::cout << std::endl << "In " << cnt << " generation" << std::endl;
	}
}

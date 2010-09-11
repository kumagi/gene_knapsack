
#include <iostream>
#include <fstream>
#include <functional>
#include <string>
#include <map>
#include <set>
#include <vector>

#include "knapsack.hpp"

#define NDEBUG
#ifndef NDEBUG
#define DEBUG(...) fprintf(stderr,__VA_ARGS__)
#define IN_DEBUG(x) x
#define GP_OUT(x)
#else
#define DEBUG(...)
#define IN_DEBUG(x)
#define GP_OUT(x) x
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
	void dump()const{
		std::cout << "random_seed: " << random_seed
							<< "\nindivisuals: " << idv_number
							<< "\nmutation rate: 1/" << mutation_rate
							<< std::endl;
	}
};
std::ostream& operator<<(std::ostream& o, const settings& s){
	o << s.idv_number << "," << s.mutation_rate;
	return o;
}


struct problem : public singleton<problem>{
	int sack_size;
	int item_quantum;
	std::vector<item> itemset;
	void dump()const{
		std::cout << "sack_size: " << sack_size << std::endl;
		std::for_each(itemset.begin(),itemset.end(),std::mem_fun_ref(&item::dump));
	}
};
std::ostream& operator<<(std::ostream& o, const problem& p){
	o << p.item_quantum;
	return o;
}



void bit_dump(bool t){
	std::cout <<( t ? "1" : "0");
}
class gene{
private:
	std::vector<bool> gen;
	bool eval_calc;
	int evaled;
public:
	gene():eval_calc(false){}
	void dump()const{
		std::cout << "[";
		std::for_each(gen.begin(),gen.end(),&bit_dump);
		std::cout << "|" << eval() << "] ";
	}
	std::vector<bool> get_gene(void)const{return gen;}
	int eval()const{
		if(eval_calc) return evaled;
		const problem& p = problem::instance();
		int ans = 0;
		int weight = 0;
		for(size_t i=0;i<gen.size();i++){
			if(gen[i]){
				ans += p.itemset[i].value();
				weight += p.itemset[i].weight();
			}
		}
		int& tmp = const_cast<int&>(evaled);
		tmp = weight > p.sack_size ? 0 : ans;
		bool& flag = const_cast<bool&>(eval_calc);
		flag = true;
		return evaled;
	}

	
	gene& operator+=(const bool& t){
		gen.push_back(t);
		return *this;
	}
	//inline void reserve(int newlength){
	//	gen.reserve(newlength/8+1);
	//}
	bool operator==(const gene& rhs)const{
		for(size_t i=0;i<gen.size();i++){
			if(gen[i] != rhs.gen[i]) return false;
		}
		return true;
	}
	bool operator!=(const gene& rhs)const{
		return !this->operator==(rhs);
	}
	bool operator<(const gene& rhs)const{
		return eval() < rhs.eval();
	}

	gene operator/(const gene& rhs)const{
		gene child;
		child.gen.reserve(gen.size());
		rand_bit rb;
		for(size_t i=0;i<gen.size();i++){
			child += rand() % settings::instance().mutation_rate ?
				(rb.get() ? gen[i] : rhs.gen[i])
				: !gen[i];
		}
		return gene(child);
	}
};
std::ostream& operator<<(std::ostream& o, const gene& g){
	o << g.eval();
	return o;
}

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
	void merge(const std::vector<gene>& org){
		for(std::vector<gene>::const_iterator it=org.begin();
				it != org.end();
				++it){
			idvs.insert(*it);
		}
	}
	void eliminate_poor(int next_size){
		assert(next_size >0);
		std::multiset<gene>::const_iterator it = idvs.begin();
		std::advance(it, next_size);
		idvs.erase(idvs.begin(),it);
	}
	bool one_gene()const{
		for(std::multiset<gene>::const_iterator it = idvs.begin();it!=idvs.end();++it){
			if(!(*idvs.begin() == *it)) return false;
		}
		//std::find_if(idvs.begin(), idvs.end(), std::bind1st(&std::equal<gene>(), *idvs.begin()));
		return true;
	}
	const gene& operator[](const size_t idx)const{
		assert(idx < idvs.size());
		std::multiset<gene>::const_iterator it = idvs.begin();
		std::advance(it, idx);
		return *it;
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
		iterator& operator++(){ ++it; return *this;}
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
		for(generation::iterator it=world.begin(); it != world.end(); ++it){
			int evaled = it->eval();
			evaled += evaled == 0 ? 1 : 0;
			sum += evaled;
			border.push_back(sum);
		}
	}
	int spin(void)const{
		const int entry = rand() % sum;
		assert(std::distance(border.begin(), std::lower_bound(border.begin(),border.end(),0)) == 0);
		return std::distance(border.begin(), std::lower_bound(border.begin(),border.end(),entry));
	}
};


int main(int argc,char** argv){
	settings& s = settings::instance();
	problem& p = problem::instance();
	{ // parse options
		if(argc < 2){fprintf(stderr,"input filename\n");exit(1);}
		s.filename = argv[1];
		DEBUG("filename: %s \n", s.filename.c_str());
		if(argc >= 3){
			s.idv_number = atoi(argv[2]);
		}else{
			s.idv_number = 512;
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
	}
	IN_DEBUG(s.dump());
	srand(s.random_seed);
	
	generation world;
	world.random_set(s.idv_number, p.item_quantum);
	//IN_DEBUG(world.sort());
	//IN_DEBUG(world.dump());
	
	// start genetic algorithm
	{
		const int qty = world.size();
		int cnt = 0;
		while(!world.one_gene()){
			const roulette rlt(world);
			std::vector<gene> new_generation;
			for(int i=0; i < qty; i++){
				const gene& parent1(world[rlt.spin()]);
				const gene& parent2(world[rlt.spin()]);
				new_generation.push_back(parent1 / parent2);
			}
			
			world.merge(new_generation);
			//std::cout << std::endl;
			world.eliminate_poor(world.size()/2);
			//world.dump();
			//std::cout<<std::endl<<std::endl;
			++cnt;
		}
		IN_DEBUG(world[0].dump());
		IN_DEBUG(std::cout << std::endl << "In " << cnt << " generation in" << world.size() << std::endl);
		GP_OUT(std::cout << p << "," << world[0] << "," << cnt << "," << s << std::endl);
	}
}

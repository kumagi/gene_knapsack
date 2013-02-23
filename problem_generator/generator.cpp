#include <stdlib.h>
#include <fstream>


int main(int argc,char** argv){
	std::string filename("items.txt");
	if(argc != 2){ fprintf(stderr,"item number required\n"); exit(1);}
	
	int item_qty;
	{// parse option
		item_qty = atoi(argv[1]);
		if(item_qty == 0) { fprintf(stderr,"wrong item number\n"); exit(1);}
	}
	std::ofstream out;
	out.open(filename.c_str());
	
	out << item_qty*100 << std::endl;
	for(int i=0; i<item_qty; i++){
		int random_value = (rand() % 200);
		int random_weight = random_value + (rand() % 20) - 10;
		out << random_value << " " << random_weight << std::endl;
	}
}

#include <stdint.h>
#include <stdio.h> //Don't need this after finished
#include <stdlib.h>//This either
#include <time.h>

#include <chrono>
#include <iostream>

typedef unsigned int uint;

//Bit Sequence Generators
inline uint64_t bitseq64(uint width){
	return -1ull >> (64-width);
}

//Nodes and Lists
struct bwr64node{
	uint64_t dat;
	bwr64node* nxt;
	
	bwr64node(){}

	bwr64node(uint64_t a){
		dat = a;
		nxt = 0;
	}
};
struct bwr64list{
	bwr64node* beg;
	bwr64node* end;
	
	bwr64list(){
		beg = 0;
		end = 0;
	}
	
	void addfront(bwr64list* a){
		if(a==0||a->beg==0) return;
		if(beg==0){
			beg = a->beg;
			end = a->end;
			return;
		}
		a->end->nxt = beg;
		beg = a->beg;
	}

	void addback(bwr64list* a){
		if(a==0||a->beg==0) return;
		if(beg==0){
			beg = a->beg;
			end = a->end;
			return;
		}
		end->nxt = a->beg;
		end = a->end;
	}

	void addfront(bwr64node* a){
		if(a==0) return;
		if(beg==0){
			beg = a;
			end = a;
			end->nxt = 0;
			return;
		}
		a->nxt = beg;
		beg = a;
	}

	void addback(bwr64node* a){
		if(a==0) return;
		if(end == 0){
			beg = a;
			end = a;
			end->nxt = 0;
			return;
		}
		a->nxt = 0;
		end->nxt = a;
		end = a;
	}

	void reset(){
		beg = 0;
		end = 0;
	}
};

//for Mode: 0 = Unsigned Integers, 1 = Signed Integers, 2 = Floating Point
void bwr64(bwr64list& list, uint sz, uint bits, uint bitstep, uint mode){
	uint64_t numlists = 1ull<<(bitstep);
	uint64_t usedlists = numlists; //Lists in use for current iteration
	bwr64list* lists = new bwr64list[numlists]();
	//bwr64list list = bwr64list();
	
	//bwr64node* nodes = new bwr64node[sz]; //SIGNIFICANTLY quicker than making them individually

	/*for(uint i = 0; i < sz; i++){ //Listify the array (if that is a word)
		nodes[i].dat = arr[i];
		nodes[i].nxt = 0;
		list.addback(&nodes[i]); //Can create these nodes into the lists when needed the first time around.
	}*/

	uint realstep = bitstep;
	bool finalstep = false;
	bwr64node* stableptr = 0; //Used to ensure stability NOT IMPLEMENTED ATM
	uint64_t bitseq = bitseq64(bitstep); //Bit sequence for determining list number
	uint64_t signdet = 0; //Sign Determinate: Used to differentiate between positive and negative on last iteration when '!unsign'
	//Main Loop
	for(uint i = 0; i < bits; i+= bitstep){
		if(i+bitstep >= bits){
			finalstep = true; //Necessary for sorting signed arrays
			realstep = bits - i;
			bitseq = bitseq64(realstep); //Update Bit Sequence, not really necessary unless 'bits' is less than max
			usedlists = 1ull<<realstep;
			signdet = usedlists>>1ull; //Properly set Sign Determinate to maxval/2
		}
		bwr64node* tmp = list.beg;
		bwr64node* tmp2 = 0;
		uint64_t ind;
		while(tmp != 0){ //Go through list
			tmp2 = tmp->nxt;
			ind = (tmp->dat>>i)&bitseq;
			if(finalstep && mode==2 && (ind&signdet)){ //Check for negative floats
				lists[ind].addfront(tmp); //Build negative lists in reverse for floats
			}else{
				lists[ind].addback(tmp); //Place in proper list
			}
			tmp = tmp2;
		}
		list.reset(); //Clear the list
		//Re-Link lists back to list
		if(finalstep && mode){ //Check for negatives in this iter
			//Re-linking strategy for the final iteration where negatives are possible requires
			//	the linking of the final half of the lists first (the negative ones).
			//
			//	Additionally, the negative lists for floats must be linked in reverse order.
			if(mode==2){//Float
				for(uint j = usedlists-1; j >= usedlists/2; j--){
					list.addback(&lists[j]);
					lists[j].reset();
				}
			}else{//Signed Ints
				for(uint j = usedlists/2; j < usedlists; j++){
					list.addback(&lists[j]);
					lists[j].reset();
				}
			}
			for(uint j = 0; j < usedlists/2; j++){ //The rest normally
				list.addback(&lists[j]);
				lists[j].reset();
			}
		}else{ //Normal Re-Link
			for(uint j = 0; j < usedlists; j++){
				list.addback(&lists[j]);
				lists[j].reset();
			}
		}
	}
	delete[] lists; //Done with these
	//Now Arrayify list back (much less likely to be a real word)
	/*bwr64node* tmp = list.beg;
	for(uint i = 0; i < sz; i++){
		arr[i] = tmp->dat;
		tmp = tmp->nxt;
	}*/
	//delete[] nodes;
	//Done
}

int main(int argc, char** argv){
	setbuf(stdin,0);
	if(argc < 2){
		printf("Either enter numbers to sort, or use '-' to use a random list\n");
		return 0;
	}
	if(argv[1][0]=='-'){
		if(argc < 6){
			printf("format: - (# of elements) (number of bits) (step size) (y/n print or not)\n");
			return 0;
		}
		int sz = atoi(argv[2]);
		int bits = atoi(argv[3]);
		int bitstep = atoi(argv[4]);
		int print = argv[5][0]=='y'?1:0;
		bwr64node* nodes = new bwr64node[sz];
		bwr64list list = bwr64list();
		srand(time(0));
		for(int i = 0; i < sz; i++){
			double tmp = (double)rand()-((double)RAND_MAX/2.0);
			nodes[i].dat = *(uint64_t*)(&tmp);
			nodes[i].nxt = 0;
			list.addback(&nodes[i]);
		}
		auto st = std::chrono::high_resolution_clock::now();
		bwr64(list,sz,bits,bitstep,2);
		auto en = std::chrono::high_resolution_clock::now();
		std::cout << "nanos: " << std::chrono::duration_cast<std::chrono::nanoseconds>(en-st).count() << '\n'
			<< "millis: " << std::chrono::duration_cast<std::chrono::milliseconds>(en-st).count() << '\n';
		bwr64node* tmp = list.beg;
		if(print)
			//for(int i = 0; i < sz; i++){
			while(tmp->nxt != 0){
				printf("%f\n",*(double*)(&(tmp->dat)));
				tmp = tmp->nxt;
			}
		delete[] nodes;
		return 0;
	}

	/*double* arr = new double[argc-1];
	for(int i = 0; i < argc-1; i++){
		arr[i] = atof(argv[i+1]);
	}
	bwr64((uint64_t*)arr,argc-1,64,8,2);

	for(int i = 0; i < argc-1; i++){
		printf("%f\n",arr[i]);
	}
	delete[] arr;*/
}

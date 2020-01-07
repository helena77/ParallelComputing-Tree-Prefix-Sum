// Sequential version of the prefix sum
// Stores interior pair-sums in a heap and interior prefix-sums in another heap

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <future>
using namespace std;

const int PARALLEL_LEVELS = 4;
typedef vector<int> Data;

class Heaper {
public:
	Heaper(const Data *data) : n(data->size()), data(data) {
		interior = new Data(n-1, 0);
	}
	virtual ~Heaper() {
		delete interior;
	}
protected:
	int n; // n is size of data, n-1 is size of interior
	const Data *data;
	Data *interior;

	virtual int size() {
		return (n-1) + n;
	}
	virtual int value(int i) {
		if (i < n-1)
			return interior->at(i);
		else
			return data->at(i - (n-1));
	}
	virtual int parent(int i) { 
		return (i-1)/2; 
	}
	virtual int left(int i) { 
		return i*2+1; 
	}
	virtual int right(int i) { 
		return left(i)+1; 
	}
	virtual bool isLeaf(int i) {
		return right(i) >= size();
	}
};

class SumHeap : public Heaper {
public:
	SumHeap(const Data *data) : Heaper(data) {
		calcSum(0, 0);
	}
	int sum(int i=0) {
		return value(i);
	}
	void prefixSums(Data *output) {
		calcPrefix(0, 0, 0, output);
	}
	void dump() {
		for (int i = 0; i < size(); i++)
			cout << i << ": " << value(i) << endl;
	}
private:
	void calcSum(int i, int level) {
		if (!isLeaf(i)) {
			if (level < PARALLEL_LEVELS) {
				auto handle = async(launch::async, &SumHeap::calcSum, this, left(i), level+1);
				calcSum(right(i), level+1);
				handle.wait();
			} else {
				calcSum(left(i), level+1);
				calcSum(right(i), level+1);
			}
			interior->at(i) = value(left(i)) + value(right(i));
		}
	}
	void calcPrefix(int i, int sumPrior, int level, Data *output) {
		if (isLeaf(i)) {
			output->at(i - (n-1)) = sumPrior + value(i);
		} else {
			if (level < PARALLEL_LEVELS) {
				auto handle = async(launch::async, &SumHeap::calcPrefix, 
									this, left(i), sumPrior, level+1, output);
				calcPrefix(right(i), sumPrior + sum(left(i)), level+1, output);
				handle.wait();
			} else {
				calcPrefix(left(i), sumPrior, level+1, output);
				calcPrefix(right(i), sumPrior + sum(left(i)), level+1, output);
			}
		}
	}
};

const int N = 1<<26;  // FIXME must be power of 2 for now

int main() {
	Data data(N, 1);  // put a 1 in each element of the data array
	Data prefix(N, 1);

	// start timer
    auto start = chrono::steady_clock::now();

	SumHeap heap(&data);
	heap.prefixSums(&prefix);
	
	// stop timer
    auto end = chrono::steady_clock::now();
    auto elpased = chrono::duration<double,milli>(end-start).count();

	int check = 1;
	for (int elem: prefix)
		if (elem != check++) {
			cout << "FAILED RESULT at " << check-1;
			break;
		}
	cout << "in " << elpased << "ms" << endl;
	return 0;
}


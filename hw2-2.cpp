/**
* @file hw2.cpp - produce an implementation of the Ladner-Fischer† 
                  parallel prefix sum algorithm
* @author Helena Wang
* @see "Seattle University, CPSC5600, Fall 2018"
*/
#include <iostream>
#include <vector>
#include <chrono>
#include <future>
using namespace std;

typedef vector<int> Data;

class Heaper {
public:
	Heaper(const Data *data) : n(data->size()), data(data) {
		interior = new Data(n - 1, 0);
	}
	virtual ~Heaper() {
		delete interior;
	}
protected:
	int n;
	const Data *data;
	Data *interior;

	/**
	 * This method is used to get the size of the array data
	 *
	**/
	virtual int size() {
		return (n - 1) + n;
	}	
	/**
	 * This method is used to return the value of the array 
	 * data element
	**/
	virtual int value(int i) {
		if (i < n - 1)
			return interior->at(i);
		else
			return data->at(i - (n - 1));
	}
	/**
	 * This method is used to check if the node is leaf
	 * or not
	**/
	virtual bool isLeaf(int i) {
		if (i < n - 1)
			return false;
		return true;
	}
	/**
	 * This method is to get the left children of the 
	 * node i
	**/
	virtual int left(int i) {
		return 2 * i + 1;
	}
	/**
	 * This method is to get the right children of the 
	 * node i
	**/
	virtual int right(int i) {
		return 2 * i + 2;
	}
	/**
	 * This method is to get the parent of the 
	 * node i
	**/
	virtual int parent(int i) {
		return (i - 1) / 2;
	}
};

class SumHeap : public Heaper {
	static const int N_THREADS = 8;     // the number of threads
public:
	SumHeap(const Data *data) : Heaper(data) {
		calcSum(0, 1);     // get the pair sum of the data
	}
	int sum(int node = 0) {
		return value(node);    // return the value of the sum
	}
	void prefixSums(Data *prefix) {
		calcPrefix(0, 0, 1, prefix);    // get the prefix sum of the data
	}
private:
	/**
	 * This method is used to calculate the pair sum of the 
	 * array data
	**/
	void calcSum(int i, int level) {
		if (isLeaf(i))
			return;
		// if the level of the node i is not larger than thread/2
		// then using the thread to do the parallel computing
		if (level <= N_THREADS/2) {
			auto handle = async(launch::async, &SumHeap::calcSum, this, left(i), level + 1);
			calcSum(right(i), level + 1);
			handle.get();
			interior->at(i) = value(left(i)) + value(right(i));  // get the interior value
			                                                     // by adding the left child
																 // value and right child value
		}
		else {
		calcSum(left(i), level + 1);
		calcSum(right(i), level + 1);
		interior->at(i) = value(left(i)) + value(right(i));
		}
	}

	/**
	 * This method is used to calculate the prefix sum of the 
	 * array data
	**/
	void calcPrefix(int i, int top, int level, Data *prefix) {
		// compute the prefix sum of the left leaves 
		if (isLeaf(i)) 
			prefix->at(i - (n - 1)) = top + value(i);
		// compute the prefix sum of interior nodes
		else if(!isLeaf(i) && level <= N_THREADS/2) {
			auto handle = async(launch::async, &SumHeap::calcPrefix, this, left(i), top, level + 1, prefix);
			calcPrefix(right(i), top + value(left(i)), level + 1, prefix);
			handle.get();
		}
		// compute the prefix sum of rest nodes
		else {
			calcPrefix(left(i), top, level + 1, prefix);
			calcPrefix(right(i), top + value(left(i)), level + 1, prefix);
		}
	}	
};


const int N = 1 << 26;  // FIXME must be power of 2 for now
int main() {
	Data data(N, 1);    // put a 1 in each element of the data array
	Data prefix(N, 1);

	// start timer
	auto start = chrono::steady_clock::now();

	SumHeap heap(&data);	
	heap.prefixSums(&prefix);
	
	// stop timer
	auto end = chrono::steady_clock::now();
	auto elpased = chrono::duration<double, milli>(end - start).count();

	int check = 1;
	for (int elem : prefix)
		if (elem != check++) {
			cout << "FAILED RESULT at " << check - 1;
			break;
		}
	cout << " in " << elpased << "ms" << endl;
	return 0;
}


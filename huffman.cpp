#include <queue>
#include <ctime>
#include <string>
#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace std;

#define SUFFIX ".huf"

struct node{
	static int count;
	node *leftSon, *rightSon, *fa;
	int weight, index;
	string bit;
	node(int index = 0){
		leftSon = rightSon = fa = NULL;
		this->index = index;
		weight = 0;
		bit = "";
	}
	bool operator <(const node& x)const{
		return weight > x.weight;
	}
}t[600];
int node::count = 256;

void dfs(node &root, string bit = ""){ //build code table for encoding
	if (root.index >= 256){ //a father node
		dfs(*root.leftSon, bit + "0");
		dfs(*root.rightSon, bit + "1");
	}else{ //leaf
		if (bit.length() == 0){ //special
			root.bit = "0";
			root.leftSon = &root; //special situation
		}else{
			root.bit = bit;
		}
	}
}

void getHuffmanTree(priority_queue<node> &q){ //construct the tree
	while (q.size() > 1){ //construct huffman tree
		int a = q.top().index;q.pop();
		int b = q.top().index;q.pop();
		int c = node::count++;
		t[c] = node(c);
		t[c].leftSon = &t[a];
		t[c].rightSon = &t[b];
		t[a].fa = t[b].fa = &t[c];
		t[c].weight = t[a].weight + t[b].weight;
		q.push(t[c]);
	}
}

bool encode(string inputFile){
	//return false if failed to open file
	//init
	priority_queue<node> q;
	unsigned char ch;
	int numOfPair = 0, numOfByte = 0;
	ifstream in(inputFile.c_str(), ios::binary);
	node::count = 256;
	//count the frequency
	while (in){
		in.read((char*)&ch, sizeof(unsigned char));
		if (t[ch].weight == 0){
			t[ch] = node((int)ch);
			++numOfPair;
		}
		if (in.eof() == true){ //eof
			break;
		}
		++t[ch].weight;
		++numOfByte;
	}
	//store the table and construct the tree
	string outputFile = inputFile + SUFFIX;
	ofstream out(outputFile.c_str(), ios::binary);
	out.write((char*)&numOfPair, sizeof(int)); //store the number of char-weight pairs
	for (int i=0;i<node::count;++i){
		if (t[i].weight > 0){ //valiable
			q.push(t[i]);
			out.write((char*)&t[i].index, sizeof(char));
			out.write((char*)&t[i].weight, sizeof(int));
		}
	}
	getHuffmanTree(q);
	//build the table
	dfs(t[q.top().index]);
	in.clear(std::ios::goodbit);
	in.seekg(std::ios::beg);
	//store the contant
	out.write((char*)&numOfByte, sizeof(int)); //original bits length of file
	unsigned char outputByte = 0;
	string temp;
	int usedBit = 0;
	while (in){
		in.read((char*)&ch, sizeof(unsigned char));
		temp = t[ch].bit; //get bit code of ch
		int n = temp.length();
		for (int i=0;i<n;++i){
			++usedBit;
			outputByte = (outputByte << 1) + (temp[i] == '0' ? 0 : 1);
			if (usedBit == 8){
				out.write((char*)&outputByte, sizeof(char));
				outputByte = usedBit = 0;
			}
		}
	}
	if (usedBit > 0){ //the rest of bits
		while (usedBit < 8){
			++usedBit;
			outputByte *= 2;
		}
		out.write((char*)&outputByte, sizeof(char));
	}
	in.close();
	out.close();
	return true;
}

bool decode(string inputFile){
	//return false if failed to open file
	int numOfPair, numOfByte, weight;
	priority_queue<node> q;
	unsigned char ch;
	ifstream in(inputFile.c_str(), ios::binary);
	ofstream out(inputFile.substr(0, inputFile.length() - strlen(SUFFIX)).c_str(), ios::binary); // cut the suffix
	if (!in || !out){ //fail to open file
		return false;
	}
	node::count = 256;
	//input char-weight table
	in.read((char*)&numOfPair, sizeof(int));
	while (numOfPair--){
		in.read((char*)&ch, sizeof(unsigned char));
		in.read((char*)&weight, sizeof(int));
		t[ch] = node(ch);
		t[ch].weight = weight;
		q.push(t[ch]);
	}
	//construct tree and get the bit string
	getHuffmanTree(q);
	int rootIndex = q.top().index;
	dfs(t[rootIndex]);
	//read file and decode
	node now = t[q.top().index];
	in.read((char*)&numOfByte, sizeof(int));
	while (numOfByte){
		in.read((char*)&ch, sizeof(char));
		for (int i=0;i<8;++i){
			int result = ch & 128; //get the highest bit
			if (result == 0){
				now = *now.leftSon;
			}else{
				now = *now.rightSon;
			}
			if (now.index < 256){ //leaves
				out.write((char*)&now.index, sizeof(char));
				now = t[rootIndex];
				--numOfByte;
				if (numOfByte == 0){
					break;
				}
			}
			ch = (ch << 1);
		}
	}
	out.close();
	in.close();
	return true;
}

int main(){
	string fileName;
	cout << "enter file name" << endl;
	cin >> fileName;
	cout << "encode or decode?(e/d)" << endl;
	getchar(); //eliminate '\n'
	char ch = getchar();
	ch = ch|32; //to lowercase
	time_t beginTime = clock();
	if (ch == 'e'){ //encode
		cout << "encoding..." << endl;
		encode(fileName);
		cout << "done" << endl;
	}else if (ch == 'd'){ //decode
		cout << "decoding..." << endl;
		decode(fileName);
		cout << "done" << endl;
	}else{ //illegal input
		//ignore
	}
	cout << "time used: " << clock() - beginTime << "ms\n";
	system("pause");
	return 0;
}

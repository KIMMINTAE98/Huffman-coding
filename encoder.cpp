#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <bitset>
#include <iomanip>
using namespace std;

class HTreeNode
{
private:
	char symbol;
	int node_count;
	int codeword_length, codeword;
	HTreeNode* rightNode, * leftNode;
public:
	HTreeNode(char c, int n) :symbol(c), node_count(n), codeword_length(0), codeword(0), rightNode(nullptr), leftNode(nullptr) {}
	int GetSymbol() { return symbol; }
	int GetNodeCount() { return node_count; }
	int GetCodewordLength() { return codeword_length; }
	int GetCodeword() { return codeword; }
	HTreeNode* GetRightnode() { return rightNode; }
	HTreeNode* GetLeftnode() { return leftNode; }
	void SetRightNode(HTreeNode* rn) { rightNode = rn; }
	void SetLeftNode(HTreeNode* ln) { leftNode = ln; }
	void PlusCodewordLength() { codeword_length++; }
	void PlusCodeword(int n) { codeword = codeword | n; }
};

// Sorting function for first huffman table
bool Compare(pair<int, HTreeNode*> a, pair<int, HTreeNode*> b)  
{
	if (a.first == b.first)                                           // if same symbol_count,
		return a.second->GetNodeCount() < b.second->GetNodeCount();   // lower subtree_size precede
	else                                                              // else,
		return a.first < b.first;                                     // lower symbol_count precede
}
// Traverse huffman subtree to append one codeword to each subtree's node 
void CodewordTraversal(HTreeNode* node, int num)
{
	node->PlusCodeword(num << node->GetCodewordLength());   // append codeword (num = 0 or 1)
	node->PlusCodewordLength();                             // codeword_length++
	if (node->GetLeftnode())                                // recursive to left subnode,
		CodewordTraversal(node->GetLeftnode(), num);        //
	if (node->GetRightnode())                               // recursive to right subnode,
		CodewordTraversal(node->GetRightnode(), num);       // 
}
// Traverse huffman tree to save leafnode's info to completed huffman table 
void ReadTraversal(HTreeNode* node, map<char, pair<int, int>>& Complete_HTable)
{
	if (node->GetSymbol())                                                                                // if leafnode,
		Complete_HTable[node->GetSymbol()] = make_pair(node->GetCodewordLength(), node->GetCodeword());   // save info(symbol, codeword_length, codeword) to huffman table
	if (node->GetLeftnode())                                                                              // recursive to left subnode,
		ReadTraversal(node->GetLeftnode(), Complete_HTable);                                              // 
	if (node->GetRightnode())                                                                             // recursive to right subnode,
		ReadTraversal(node->GetRightnode(), Complete_HTable);                                             // 
}
// Traverse huffman tree to delete memory 
void DeleteTraversal(HTreeNode* node)
{
	if (node->GetLeftnode())                     // recursive to left subnode,
		DeleteTraversal(node->GetLeftnode());    // 
	if (node->GetRightnode())                    // recursive to right subnode,
		DeleteTraversal(node->GetRightnode());   //
	delete node;                                 // delete HTreeNode memory
}

int main()
{
	char symbol;
	map<char, int> symbol_count;
	ifstream fin("input_data.txt");                 // Open input file 
	if (!fin.is_open()) {                           // if open fail, 
		cout << "input file open error!" << endl;   // print error message
		exit(-1);                                   // and exit program
	}
	while (fin)                   // Read input file 
	{                             //
		symbol = fin.get();       // read one character
		symbol_count[symbol]++;   // count that character
	}                             
	fin.close();   // close input file

	vector<pair<int,HTreeNode*>> HTable;                                   // Make first huffman table
	for (auto it = symbol_count.begin(); it != symbol_count.end(); it++)   // 
	{                                                                      // 
		HTreeNode* newNode = new HTreeNode(it->first, 1);                  // make huffman tree leaf node
		HTable.push_back(make_pair(it->second, newNode));                  // add huffman table entry(symbol_count, huffman tree node)
	}
	for (int i = 0; i < symbol_count.size() - 1; i++)                                                                    // Make huffman tree
	{                                                                                                                    // 
		sort(HTable.begin(), HTable.end(), Compare);                                                                     // sort huffman table to symbol_count(subtree_size) order 
		HTreeNode* newNode = new HTreeNode(NULL, HTable[0].second->GetNodeCount() + HTable[1].second->GetNodeCount());   // make new subtree (internal node's symbol = NULL)
		newNode->SetLeftNode(HTable[0].second);                                                                          // set left subnode to first huffman table entry's subtree
		newNode->SetRightNode(HTable[1].second);                                                                         // set right subnode to second huffman table entry's subtree
		CodewordTraversal(HTable[0].second, 0);                                                                          // add 0 to left subtree's codeword
		CodewordTraversal(HTable[1].second, 1);                                                                          // add 1 to right subtree's codeword
		HTable.insert(HTable.begin(), make_pair(HTable[0].first + HTable[1].first, newNode));                            // add new entry to huffman table
		HTable.erase(HTable.begin() + 1);                                                                                // delete first entry in huffman table
		HTable.erase(HTable.begin() + 1);                                                                                // delete second entry in huffman table
	}
	map<char, pair<int, int>> Complete_HTable;          // Make completed huffman table
	ReadTraversal(HTable[0].second, Complete_HTable);   // save each huffman tree leafnode's info to new huffman table entry

	ofstream fout("huffman_table.hbs", ios::binary);       // Open output table file 
	if (!fout.is_open()) {                                 // if open fail, 
		cout << "output table file open error!" << endl;   // print error message
		DeleteTraversal(HTable[0].second);                 // delete htree memory
		exit(-1);                                          // and exit program
	}
	char buf = 0;                                                                                                    // Write Huffman_table.hbs file
	int remain_bit = 8;                                                                                              // (write data in 1byte buffer unit)
	for (auto it = Complete_HTable.begin(); it != Complete_HTable.end(); it++)                                       // loop in huffman table entry
	{                                                                                                                // 
		int codeword_length = it->second.first;                                                                      // save codeword_length
		unsigned int curText = it->first;                                                                            // save current entry bitstream to write using shift
		curText = (curText << (codeword_length + 8)) | (codeword_length << codeword_length) | (it->second.second);   // = [symbol(8bit), codeword_length(8bit), codeword(codeword_length bit)]
		curText = curText << (32 - codeword_length + 16);                                                            // left align bitstream in 32bit
		for (int i = codeword_length + 16; i > 0; )                                                                  // loop untill write all bitstream
		{                                                                                                            // 
			buf = buf | (curText >> (32 - remain_bit));                                                              // fill buffer first
			if (i >= remain_bit) {                                                                                   // if remain bitstream >= remain bits in 1byte buffer(= buffer is full), 
				i -= remain_bit;                                                                                     // remain bitstream size -= remain bits in buffer
				curText = curText << remain_bit;                                                                     // remove bitstream which went to buffer
				fout << buf;                                                                                         // write buffer to file
				buf = 0;                                                                                             // reset buffer
				remain_bit = 8;                                                                                      // 
			}                                                                                                        // 
			else {                                                                                                   // if remain bitstream < remain bits in 1byte buffer(= buffer isn't full), 
				remain_bit -= i;                                                                                     // remain bits in buffer -= remain bitstream size
				i = 0;                                                                                               // end loop
			}                                                                                                        // 
		}                                                                                                            // 
	}                                                                                                                // 
	fout << buf;                                                                                                     // write last buffer
	fout.close();                                                                                                    // close output table file 

	fin.open("input_data.txt");                           // Open input file 
	fout.open("huffman_code.hbs", ios::binary);           // Open output code file 
	if (!fin.is_open()) {                                 // if input file open fail, 
		cout << "input file open error!" << endl;         // print error message
		DeleteTraversal(HTable[0].second);                // delete htree memory
		exit(-1);                                         // and exit program
	}                                                     //
	if (!fout.is_open()) {                                // if output code file open fail, 
		cout << "output code file open error!" << endl;   // print error message
		DeleteTraversal(HTable[0].second);                // delete htree memory
		exit(-1);                                         // and exit program
	}
	buf = 0;                                                     // Write huffman_code.hbs file
	remain_bit = 8;                                              // (write data in 1byte buffer unit)
	while (fin)                                                  // read input file 
	{                                                            //
		symbol = fin.get();                                      // read one character
		int codeword_length = Complete_HTable[symbol].first;     // save codeword_length
		unsigned int curText = Complete_HTable[symbol].second;   // save current codeword to write file
		curText = curText << (32 - codeword_length);             // left align codeword in 32bit
		for (int i = codeword_length; i > 0; )                   // loop untill write all codeword
		{                                                        //
			buf = buf | (curText >> (32 - remain_bit));          // fill buffer first
			if (i >= remain_bit) {                               // if remain bitstream >= remain bits in 1byte buffer(= buffer is full), 
				i -= remain_bit;                                 // remain bitstream size -= remain bits in buffer
				curText = curText << remain_bit;                 // remove bitstream which went to buffer
				fout << buf;                                     // write buffer to file
				buf = 0;                                         // reset buffer
				remain_bit = 8;                                  //
			}                                                    //
			else {                                               // if remain bitstream < remain bits in 1byte buffer(= buffer isn't full),
				remain_bit -= i;                                 // remain bits in buffer -= remain bitstream size
				i = 0;                                           // end loop
			}                                                    //
		}                                                        //
	}                                                            //
	fout << buf;                                                 // write last buffer
	fin.close();                                                 // close input file 
	fout.close();                                                // close output code file 

	cout << "SYMBOL" << setw(7) << "COUNT" << setw(10);
	cout << "LENGTH" << setw(17) << "CODEWORD" << endl;
	for (auto it = Complete_HTable.begin(); it != Complete_HTable.end(); it++)
	{
		bitset<32> codeword = it->second.second;
		cout << "'" << it->first << "'" << setw(10);
		cout << symbol_count[it->first] << setw(10) << it->second.first << setw(10);
		for (int i = it->second.first - 1; i >= 0; i--)
			cout << codeword[i];
		cout << endl;
	}
	DeleteTraversal(HTable[0].second);
	return 0;
}

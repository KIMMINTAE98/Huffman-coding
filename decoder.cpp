#include <iostream>
#include <fstream>
#include <map>
#include <bitset>
#include <iomanip>
using namespace std;

int main()
{
	ifstream fin("huffman_table.hbs", ios::binary);         // Open huffman table file
	if (!fin.is_open()) {                                   // if huffman table file open fail, 
		cout << "huffman table file open error!" << endl;   // print error message
		exit(-1);                                           // and exit program
	}
	char symbol, codeword_length;                                   // Make huffman table from Huffman_table.hbs                                 
	int codeword;                                                   // 
	map<char, pair<char, int>> HTable;                              // 
	unsigned char buf = fin.get();                                  // (read data in 1byte buffer unit)
	int remain_bit = 8;                                             // 
	while (1)                                                       // read huffman table file
	{                                                               //
		symbol = 0;                                                 // reset symbol
		codeword_length = 0;                                        // reset codeword_length
		codeword = 0;                                               // reset codeword
		for (int i = 8; i > 0;)                                     // decode symbol (8bit)
		{                                                           // 
			symbol = symbol | (buf >> (8 - i));                     // save symbol from buffer first
			if (i >= remain_bit) {                                  // if remain decode bits >= remain bits in 1byte buffer(= buffer will be emtpy),
				i -= remain_bit;                                    // remain decode bits -= remain bits in buffer
				buf = fin.get();                                    // refill buffer
				remain_bit = 8;                                     // 
			}                                                       // 
			else {                                                  // if remain decode bits < remain bits in 1byte buffer(= buffer won't be empty), 
				buf = buf << i;                                     // remove buffer bits which are saved to symbol
				remain_bit -= i;                                    // remain bits in buffer -= remain decode bits
				i = 0;                                              // decode symbol end 
			}
		}
		if (!fin)                                                   // if read all huffman table file,
			break;                                                  // decode end
		for (int i = 8; i > 0;)                                     // decode codeword_length (8bit)
		{                                                           // 
			codeword_length = codeword_length | (buf >> (8 - i));   // save codeword_length from buffer first
			if (i >= remain_bit) {                                  // if remain decode bits >= remain bits in 1byte buffer(= buffer will be emtpy),
				i -= remain_bit;                                    // remain decode bits -= remain bits in buffer
				buf = fin.get();                                    // refill buffer
				remain_bit = 8;                                     //
			}                                                       // 
			else {                                                  // if remain decode bits < remain bits in 1byte buffer(= buffer won't be empty), 
				buf = buf << i;                                     // remove buffer bits which are saved to codeword_length
				remain_bit -= i;                                    // remain bits in buffer -= remain decode bits
				i = 0;                                              // decode codeword_length end
			}
		}
		for (int i = codeword_length; i > 0;)                       // decode codeword (codeword_length bit)
		{                                                           // 
			if (i >= remain_bit) {                                  // if remain decode bits >= remain bits in 1byte buffer(= buffer will be emtpy),
				codeword = codeword << remain_bit;                  // adjust bit
				codeword = codeword | (buf >> (8 - remain_bit));    // save codeword from buffer
				i -= remain_bit;                                    // remain decode bits -= remain bits in buffer
				buf = fin.get();                                    // refill buffer
				remain_bit = 8;                                     // 
			}                                                       // 
			else {                                                  // if remain decode bits < remain bits in 1byte buffer(= buffer won't be empty), 
				codeword = codeword << i;                           // adjust bit
				codeword = codeword | (buf >> (8 - i));             // save codeword from buffer
				buf = buf << i;                                     // remove buffer bits which are saved to codeword_length
				remain_bit -= i;                                    // remain bits in buffer -= remain decode bits
				i = 0;                                              // decode codeword end
			}
		}
		HTable[symbol] = make_pair(codeword_length, codeword);
	}
	fin.close();   // close huffman table file

	fin.open("huffman_code.hbs", ios::binary);             // Open huffman code file
	ofstream fout("output.txt");                           // Open output file
	if (!fin.is_open()) {                                  // if huffman code file open fail, 
		cout << "huffman code file open error!" << endl;   // print error message
		exit(-1);                                          // and exit program
	}
	if (!fout.is_open()) {                                 // if output file open fail, 
		cout << "output file open error!" << endl;         // print error message
		exit(-1);                                          // and exit program
	}
	unsigned int bitstream = fin.get();                                          // Decode huffman_code.hbs file
	bitstream = (bitstream << 8) | fin.get();                                    // read and fill 32bits bitstream first
	bitstream = (bitstream << 8) | fin.get();                                    // 
	bitstream = (bitstream << 8) | fin.get();                                    // 
	int bitlength = 32;                                                          // = remain encoded bit in bitstream 
	while (fin)                                                                  // read all encoded file
	{                                                                            // 
		for (auto it = HTable.begin(); it != HTable.end(); it++)                 // check bitstream with huffman table
		{                                                                        // 
			if ((bitstream >> (32 - it->second.first)) == it->second.second) {   // if find codeword,
				fout << it->first;                                               // write symbol to output file
				bitstream = bitstream << it->second.first;                       // remove written bit in bitstream
				bitlength -= it->second.first;                                   // 
				break;                                                           // 
			}                                                                    // 
		}                                                                        // 
		if (bitlength <= 16) {                                                   // if bitstream <= 16bits,
		    int temp = fin.get();                                                // fill 16bits
			bitstream = bitstream | (temp << (16 - bitlength + 8));              // 
			temp = fin.get();                                                    // 
			bitstream = bitstream | (temp << (16 - bitlength));                  // 
			bitlength += 16;                                                     //
		}                                                                        
	}
	fin.close();    // close huffman code file
	fout.close();   // close output file

	cout << "SYMBOL" << setw(10) << "LENGTH" << setw(14) << "CODEWORD" << endl;
	for (auto it = HTable.begin(); it != HTable.end(); it++)
	{
		bitset<32> codeword = it->second.second;
		cout << "'" << it->first << "'" << setw(10);
		cout << (int)it->second.first << setw(10);
		for (int i = it->second.first - 1; i >= 0; i--)
			cout << codeword[i];
		cout << endl;
	}
	return 0;
}

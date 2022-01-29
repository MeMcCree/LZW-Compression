#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

using namespace std;

struct DictEntry {
	unsigned int uRef;
	char cAddSym;
	string sVal;
};

void compress(string sInFile, string sOutFile) {
	vector <DictEntry> dict;
	ifstream in(sInFile, ios::binary);

	in.seekg(0, in.end);
	unsigned int uLn = in.tellg();
	in.seekg(0, in.beg);

	char* buffer = new char[uLn];
	in.read(buffer, uLn);
	in.close();

	dict.push_back({0, '\0', ""});
	string sPrefix;

	int iPrevRet = -1;
	for (unsigned int i = 0; i < uLn; i++) {
		sPrefix += buffer[i];
		
		int iRet = -1;
		for (int i = 0; i < dict.size(); i++) {
			if (dict[i].sVal == sPrefix) {
				iRet = i;
				break;
			}
		}

		if (iRet < 0) {
			unsigned int ref = (iPrevRet < 0 ? 0 : iPrevRet);
			dict.push_back({ref, buffer[i], dict[ref].sVal + buffer[i]});
			sPrefix = "";
		}
		iPrevRet = iRet;
	}
	delete[] buffer;

	char lg = ceil(log2(dict.size() - 1));
	if (lg % 8) lg = lg / 8 + 1;
	else				lg = lg / 8;

	ofstream out(sOutFile, ios::binary);
	out.write((char*)(&lg), 1);
	for (unsigned int i = 1; i < dict.size(); i++) {
#ifdef _DEBUG
		printf("%u: (%u, %c) | %s\n", i, dict[i].uRef, dict[i].cAddSym, dict[i].sVal.c_str());
#endif
		out.write((char*)(&dict[i].uRef), lg);
		out.put(dict[i].cAddSym);
	}
}

int main(int argc, char* argv[]) {
	if (argc < 3) return 1;
	compress(argv[1], argv[2]);

	return 0;
}
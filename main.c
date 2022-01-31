#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include "vector/vector.h"

const uint64_t HASH0_P = 151ul;
const uint64_t HASH0_M = 100000004987ul;

const uint64_t HASH1_P = 127ul;
const uint64_t HASH1_M = 100000000817ul;

typedef struct {
	uint64_t p, val;
} Hash;

typedef struct {
	uint32_t ref;
	char addSym;
	Hash hashes[2];
} DictEntry;

DECLARE_VECTOR(DictEntry)

Vec_DictEntry dict;
DictEntry curEntry;

int32_t find_eq() {
	DictEntry entry;
	for (int32_t i = 0; i < dict.size; i++) {
		entry = dict.Get(&dict, i);
		if (entry.hashes[0].p 	!= curEntry.hashes[0].p) 		continue;
		if (entry.hashes[0].val != curEntry.hashes[0].val) 	continue;

		if (entry.hashes[1].p 	!= curEntry.hashes[1].p) continue;
		if (entry.hashes[1].val != curEntry.hashes[1].val) continue;

		return i;
	}
	return -1;
}

uint32_t sz;
char vals[65536][1024];

void compress_file(char* inpf, char* outf) {
	sz = 1;
	VecInit_DictEntry(&dict);
	curEntry.hashes[0].p = 1; curEntry.hashes[0].val = 0;
	curEntry.hashes[1].p = 1; curEntry.hashes[1].val = 0;

	FILE* fp = fopen(inpf, "r");
	fseek(fp, 0, SEEK_END);
  uint32_t fsize = ftell(fp);
  rewind(fp);
  char* s = malloc(fsize);
  fread(s, fsize, 1, fp);

  dict.Push(&dict, curEntry);
	int32_t prevRet = -1;
	for (uint32_t i = 0; i < fsize; i++) {
		curEntry.hashes[0].val += (s[i] * curEntry.hashes[0].p) % HASH0_M;
		curEntry.hashes[0].p = (curEntry.hashes[0].p * HASH0_P) % HASH0_M;

		curEntry.hashes[1].val += (s[i] * curEntry.hashes[1].p) % HASH1_M;
		curEntry.hashes[1].p = (curEntry.hashes[1].p * HASH1_P) % HASH1_M;

		int32_t ret = find_eq();
		if (ret == -1) {
			curEntry.ref = (prevRet < 0 ? 0 : prevRet);
			curEntry.addSym = s[i];
			dict.Push(&dict, curEntry);
#ifdef _DEBUG
			strcpy(vals[sz], vals[curEntry.ref]);
			uint32_t ln = strlen(vals[sz]);
			vals[sz][ln] = curEntry.addSym; vals[sz][ln + 1] = '\0';
#endif
			sz++;

			curEntry.hashes[0].p = 1; curEntry.hashes[0].val = 0;
			curEntry.hashes[1].p = 1; curEntry.hashes[1].val = 0;
		}
		prevRet = ret;
	}

	if (prevRet != -1) {
		curEntry.ref = 0;
		curEntry.addSym = s[fsize - 1];
		dict.Push(&dict, curEntry);
	}

	fp = fopen(outf, "wb");

	uint8_t lg = ceil(log2(sz));
	if (lg % 8) lg = lg / 8 + 1;
	else				lg = lg / 8;

	fwrite(&lg, sizeof(uint8_t), 1, fp);
	for (uint32_t i = 1; i < dict.size; i++) {
		curEntry = dict.Get(&dict, i);
#ifdef _DEBUG
		printf("%i: (%i, %c) | %s\n", i, curEntry.ref, curEntry.addSym, vals[i]);
#endif
		fwrite((char*)(&curEntry.ref), lg, 1, fp);
		fwrite(&curEntry.addSym, sizeof(char), 1, fp);
	}
	VecFree_DictEntry(&dict);

	fclose(fp);
}

void decompress_file(char* inpf, char* outf) {
	sz = 1;
	FILE* infp = fopen(inpf, "rb");
	FILE* outfp = fopen(outf, "w");

	uint8_t lg;
	fread(&lg, sizeof(uint8_t), 1, infp);

	uint32_t ref; char addSym;
	while (ref = 0, fread((char*)(&ref), lg, 1, infp)) {
		fread(&addSym, sizeof(char), 1, infp);

		strcpy(vals[sz], vals[ref]);
		int32_t ln = strlen(vals[sz]);
		vals[sz][ln] = addSym; vals[sz][ln + 1] = '\0';
		fprintf(outfp, "%s", vals[sz]);
#ifdef _DEBUG
		printf("%i: (%u, %c) | %s\n", sz, ref, addSym, vals[sz]);
#endif
		sz++;
	}

	fclose(infp);
	fclose(outfp);
}

int32_t main(int32_t argc, char* argv[]) {
	if (argc != 4) return 1;

	char* mode = argv[1];

	if (strcmp(mode, "c") == 0) {
		compress_file(argv[2], argv[3]);
	} else if (strcmp(mode, "d") == 0) {
		decompress_file(argv[2], argv[3]);
	} else {
		return 1;
	}

	return 0;
}

/*
Using git submodules:
Add submodule:
	git submodule add <remote_url> <destination_folder>
Init submodule:
	git submodule update --init --recursive
Update submodule:
	git submodule update --remote --merge
*/
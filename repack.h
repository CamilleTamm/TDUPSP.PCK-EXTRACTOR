#ifndef REPACK_H
#define REPACK_H

#include <string>

const unsigned char MHPF_TAG[8] = { 'M', 'H', 'P', 'F', 1, 0, 0, 0 };

struct FILE_
{
	unsigned long unknow;
	unsigned long offset;
	unsigned long size;

	unsigned short name_size;
	char *name;
	
	unsigned long size_mod;
};

void repack();

void repack_add(unsigned long, std::string);

#endif

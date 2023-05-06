#include "stdlib.h"
#include "stdio.h"
#include "windows.h"
#include <dirent.h>
#include <errno.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "repack.h"

std::vector<std::string> split(char *str, unsigned short length, char sep)
{
	std::vector<std::string> v;
	std::string curr = "";

	// -1 car last char = 0
	for(unsigned short i = 0; i < length-1; i++)
	{
		if(str[i] == sep)
		{
			v.push_back(curr);
			curr = "";
		}
		else
		{
			curr += str[i];
		}
	}

	if(curr != "")
		v.push_back(curr);

	return v;
}

int main()
{
	FILE*f = fopen("TDUPSP.PCK", "rb");

	if(!f)
		return 1;

	//MHPF10000
	fseek(f, 8, SEEK_SET);

	unsigned long fileSize = 0;
	fread(&fileSize, sizeof(unsigned long), 1, f);

	unsigned long nbPackedFiles = 0;
	fread(&nbPackedFiles, sizeof(unsigned long), 1, f);

	FILE_ files[nbPackedFiles];

	// nombre de file type surement (laisser à 31)
	unsigned long maybeNbPackedFolders = 0;
	fread(&maybeNbPackedFolders, sizeof(unsigned long), 1, f);

	unsigned long section1Offset = 0;
	fread(&section1Offset, sizeof(unsigned long), 1, f);

	unsigned long section1Size = 0;
	fread(&section1Size, sizeof(unsigned long), 1, f); // 12 * nbPackedFiles

	unsigned long firstFileOffset = 0;
	fread(&firstFileOffset, sizeof(unsigned long), 1, f);

	// nameLenghtSection - firstFileOffset (en gros size occupé par les files)
	unsigned long SIZE_1 = 0;
	fread(&SIZE_1, sizeof(unsigned long), 1, f);

	// name 1 offset puis name 2 offset puis name 3 offset etc...
	unsigned long nameLenghtSection = 0;
	fread(&nameLenghtSection, sizeof(unsigned long), 1, f);

	unsigned long nameLenghtSectionSize = 0;
	fread(&nameLenghtSectionSize, sizeof(unsigned long), 1, f); // 4 * ,nbPackedFiles

	unsigned long nameSectionOffset = 0;
	fread(&nameSectionOffset, sizeof(unsigned long), 1, f);

	unsigned long nameSectionSize = 0;
	fread(&nameSectionSize, sizeof(unsigned long), 1, f);

	fseek(f, section1Offset, SEEK_SET);

	for(unsigned long i = 0; i < nbPackedFiles; i++)
	{
		unsigned long unknow = 0, fileOffset = 0, fileSize = 0;

		fread(&unknow, sizeof(unsigned long), 1, f); // id du fichier ! important car
		// si pour ford gt enlevé alors la voiture disparait...
		fread(&fileOffset, sizeof(unsigned long), 1, f);
		fread(&fileSize, sizeof(unsigned long), 1, f);

		files[i] = {unknow, fileOffset, fileSize};

		// std::cout << unknow << std::endl;
		// getchar();
	}

	fseek(f, nameLenghtSection, SEEK_SET);
	std::vector<unsigned long> nameOffsetProgress;

	for(unsigned long i = 0; i < nbPackedFiles; i++)
	{
		unsigned long j = 0;
		fread(&j, sizeof(unsigned long), 1, f);
		nameOffsetProgress.push_back(j);
	}

	// fseek(f, nameSectionOffset, SEEK_SET);
	if(ftell(f) != (long)nameSectionOffset)
	{
		return 1;
	}

	for(unsigned long i = 0; i < nbPackedFiles; i++)
	{
		unsigned short k = 0;

		if(i < nbPackedFiles-1)
			k = (unsigned short)(nameOffsetProgress[i+1] - nameOffsetProgress[i]);
		else
			k = (unsigned short)(fileSize - nameOffsetProgress[i]);

		files[i].name_size = k;

		files[i].name = new char[k];
		fread(files[i].name, sizeof(char), k, f);

		repack_add(files[i].unknow, files[i].name);
	}

	for(unsigned long i = 0; i < nbPackedFiles; i++)
	{
		std::vector<std::string> v = split(files[i].name, files[i].name_size, '/');

		if(v.size() > 1)
		{
			std::string curr = "extracted";

			for(unsigned int j = 0; j < v.size()-1; j++)
			{
				curr += '/' + v[j];

				DIR* dir = opendir(curr.c_str());

				if (dir)
				{
					/* Directory exists. */
					closedir(dir);
				}
				else if (ENOENT == errno)
				{
					/* Directory does not exist. */
					CreateDirectory(curr.c_str(), NULL);
				}
				else
				{
					/* opendir() failed for some other reason. */
					std::cout << ENOENT << std::endl;
				}
			}
		}

		v.clear();
	}

	for(unsigned long i = 0; i < nbPackedFiles; i++)
	{
		fseek(f, files[i].offset, SEEK_SET);
		unsigned char *data = new unsigned char[files[i].size];
		fread(data, sizeof(unsigned char), files[i].size, f);

		std::string fullPath = "extracted/" + std::string((char*)files[i].name);

		if(fullPath == "extracted/euro/bnk/vehicules/ford_gt.bnk")
			std::cout << files[i].unknow << std::endl;
		
		// FILE *f2 = fopen(fullPath.c_str(), "wb");

		// if(f2)
		// {
			// fwrite(data, sizeof(unsigned char), files[i].size, f2);
			// fclose(f2);
		// }

		delete [] data;
	}

	fclose(f);

	// repack();
	
	std::cout << "done !";
	getchar();

    return 0;
}

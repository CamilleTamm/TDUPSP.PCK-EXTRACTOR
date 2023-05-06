#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <vector>

#include "repack.h"

std::vector<FILE_> files;
unsigned long totPackedSize = 0;
unsigned long totNameSize = 0;
const unsigned long unknow1 = 31;

std::vector<unsigned long> a_;
std::vector<std::string> a__;

void repack_add(unsigned long b_, std::string b__)
{
	a_.push_back(b_);
	a__.push_back(b__);
}

void listDir(const char* path)
{
	DIR* dir;
	struct dirent *ent;

	if((dir=opendir(path)) != NULL)
	{
		while (( ent = readdir(dir)) != NULL)
		{
			if(strcmp(ent->d_name, ".") != 0  && strcmp(ent->d_name, "..") != 0)
			{
				// printf("%s\n", ent->d_name);

				std::string next = std::string((char*)path) + '/' + std::string((char*)ent->d_name);

				// listDir(ent->d_name);
				listDir(next.c_str());
			}
		}

		closedir(dir);
	}
	else
	{
		// printf("%s\n", path);

		FILE*f = fopen(path, "rb");
		long size = 0;

		if(f)
		{
			fseek(f, 0, SEEK_END); // seek to end of file
			size = ftell(f);
			fclose(f);
		}
		else
		{
			std::cout << "failed to open " << path << std::endl;
		}

		FILE_ file_;

		file_.size = (unsigned long)size;
		totPackedSize += (unsigned long)size;

		file_.size_mod = file_.size;
		while(file_.size_mod % 16 != 0) file_.size_mod++;

		std::string name;
		for(unsigned int i = 10; i < strlen(path); i++)
			name += path[i];

		file_.name_size = (unsigned short)(name.length()+1);
		totNameSize += (unsigned long)(name.length()+1);
		file_.name = new char[file_.name_size];
		strcpy(file_.name, name.c_str());

		// std::cout << file_.name << '|' << file_.size << std::endl;
		// getchar();
		files.push_back(file_);
	}
}

void repack()
{
	listDir("extracted");

	FILE*f = fopen("new_TDUPSP.PCK", "wb");

	if(f)
	{
		unsigned char empty[65536];
		for(int i = 0; i < 65536; i++) empty[i] = 0;


		fwrite(MHPF_TAG, sizeof(unsigned char), 8, f);

		// pour l'instant manuellement
		unsigned long fileSize = 0;
		fwrite(&fileSize, sizeof(unsigned long), 1, f);

		unsigned long nbPackedFiles = (unsigned long)files.size();
		fwrite(&nbPackedFiles, sizeof(unsigned long), 1, f);

		fwrite(&unknow1, sizeof(unsigned long), 1, f); // 31

		unsigned long section1off = 2048;
		fwrite(&section1off, sizeof(unsigned long), 1, f);

		fwrite(empty, sizeof(unsigned char), 2024, f);

		unsigned long curr_off = 59392;

		for(unsigned int j = 0; j < a__.size(); j++)
		{ bool found_ = false;
			for(unsigned long i = 0; i < files.size(); i++)
			{
				if(std::string((char*)files[i].name) == a__[j])
				{
					// fwrite(&i, sizeof(unsigned long), 1, f);
					fwrite(&a_[j], sizeof(unsigned long), 1, f);

					fwrite(&curr_off, sizeof(unsigned long), 1, f);
					fwrite(&files[i].size, sizeof(unsigned long), 1, f);

					curr_off += files[i].size_mod;
					found_ = true;
					break;
				}
			}
			
			if(!found_)
				std::cout << "not found " << a__[j];
		}

		fwrite(empty, sizeof(unsigned char), 57344 - (files.size() * 12), f);

		for(unsigned int j = 0; j < a__.size(); j++)
		{
			for(unsigned long i = 0; i < files.size(); i++)
			{
				if(std::string((char*)files[i].name) == a__[j])
				{
					std::string path = "extracted/" + std::string((char*)files[i].name);

					FILE*f2 = fopen(path.c_str(), "rb");

					if(f2)
					{
						unsigned char *data = new unsigned char[files[i].size];

						fread(data, sizeof(unsigned char), files[i].size, f2);

						fwrite(data, sizeof(unsigned char), files[i].size, f);
						fwrite(empty, sizeof(unsigned char), files[i].size_mod - files[i].size, f);

						delete [] data;
						fclose(f2);
					}
					
					break;
				}
			}
		}

		fclose(f);
	}
}

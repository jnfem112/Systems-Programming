#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <dirent.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include "loser.h"
#include "MD5.h"
using namespace std;

bool compare_copied(const pair <string , string> &copied_1 , const pair <string , string> &copied_2)
{
	return copied_1.first < copied_2.first;
}

void list_file(vector <string> &file_list)
{
	string file_name;
	struct dirent *entry;
	DIR *directory = opendir(".");

	while ((entry = readdir(directory)) != 0)
	{
		file_name = entry->d_name;
		if (file_name != "." && file_name != ".." && file_name != ".loser_record")
			file_list.push_back(file_name);
	}

	sort(file_list.begin() , file_list.end());

	closedir(directory);
	return;
}

void list_commit_size(FILE *loser_record , vector <uint32_t> &commit_size_list)
{
	uint32_t commit_size;

	fseek(loser_record , 0 , SEEK_SET);

	while (true)
	{
		fseek(loser_record , 6 * sizeof(uint32_t) , SEEK_CUR);
		if (!fread(&commit_size , sizeof(uint32_t) , 1 , loser_record))
			break;
		commit_size_list.push_back(commit_size);
		fseek(loser_record , commit_size - 7 * sizeof(uint32_t) , SEEK_CUR);
	}

	return;
}

uint32_t calculate_commit_size(vector <string> &new_file , vector <string> &modified , vector <pair <string , string> > &copied , vector <string> &deleted , vector <string> &file_list)
{
	uint32_t number_of_file , number_of_add , number_of_modify , number_of_copy , number_of_delete;
	char file_name[MAX_LENGTH_OF_FILE_NAME] = {0};

	uint32_t commit_size = 7 * sizeof(uint32_t);

	number_of_add = new_file.size();
	for (uint32_t i = 0 ; i < number_of_add ; i++)
	{
		strcpy(file_name , new_file[i].c_str());
		commit_size += sizeof(uint8_t) + strlen(file_name) * sizeof(char);
	}

	number_of_modify = modified.size();
	for (uint32_t i = 0 ; i < number_of_modify ; i++)
	{
		strcpy(file_name , modified[i].c_str());
		commit_size += sizeof(uint8_t) + strlen(file_name) * sizeof(char);
	}

	number_of_copy = copied.size();
	for (uint32_t i = 0 ; i < number_of_copy ; i++)
	{
		strcpy(file_name , copied[i].first.c_str());
		commit_size += sizeof(uint8_t) + strlen(file_name) * sizeof(char);
		strcpy(file_name , copied[i].second.c_str());
		commit_size += sizeof(uint8_t) + strlen(file_name) * sizeof(char);
	}

	number_of_delete = deleted.size();
	for (uint32_t i = 0 ; i < number_of_delete ; i++)
	{
		strcpy(file_name , deleted[i].c_str());
		commit_size += sizeof(uint8_t) + strlen(file_name) * sizeof(char);
	}

	number_of_file = file_list.size();
	for (uint32_t i = 0 ; i < number_of_file ; i++)
	{
		strcpy(file_name , file_list[i].c_str());
		commit_size += sizeof(uint8_t) + strlen(file_name) * sizeof(char) + LENGTH_OF_MD5 * sizeof(uint8_t);
	}

	return commit_size;
}

void list_commit(FILE *loser_record , uint32_t index , vector <uint32_t> &commit_size_list , unordered_map <string , string> &file_list)
{
	uint32_t offset = 0;
	uint32_t number_of_commit , number_of_file , number_of_add , number_of_modify , number_of_copy , number_of_delete , commit_size;
	uint8_t file_name_size;
	char file_name[MAX_LENGTH_OF_FILE_NAME] = {0};
	uint8_t MD5[LENGTH_OF_MD5 + 1] = {0};

	for (uint32_t i = 0 ; i < index - 1 ; i++)
		offset += commit_size_list[i];
	fseek(loser_record , offset , SEEK_SET);

	fread(&number_of_commit , sizeof(uint32_t) , 1 , loser_record);
	fread(&number_of_file , sizeof(uint32_t) , 1 , loser_record);
	fread(&number_of_add , sizeof(uint32_t) , 1 , loser_record);
	fread(&number_of_modify , sizeof(uint32_t) , 1 , loser_record);
	fread(&number_of_copy , sizeof(uint32_t) , 1 , loser_record);
	fread(&number_of_delete , sizeof(uint32_t) , 1 , loser_record);
	fread(&commit_size , sizeof(uint32_t) , 1 , loser_record);

	for (uint32_t i = 0 ; i < number_of_add ; i++)
	{
		fread(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fread(file_name , sizeof(char) , file_name_size , loser_record);
	}

	for (uint32_t i = 0 ; i < number_of_modify ; i++)
	{
		fread(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fread(file_name , sizeof(char) , file_name_size , loser_record);
	}

	for (uint32_t i = 0 ; i < number_of_copy ; i++)
	{
		fread(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fread(file_name , sizeof(char) , file_name_size , loser_record);
		fread(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fread(file_name , sizeof(char) , file_name_size , loser_record);
	}

	for (uint32_t i = 0 ; i < number_of_delete ; i++)
	{
		fread(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fread(file_name , sizeof(char) , file_name_size , loser_record);
	}

	for (uint32_t i = 0 ; i < number_of_file ; i++)
	{
		fread(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fread(file_name , sizeof(char) , file_name_size , loser_record);
		fread(MD5 , sizeof(uint8_t) , LENGTH_OF_MD5 , loser_record);
		file_list[file_name] = (char *)MD5;
	}

	return;
}

void print_commit(FILE *loser_record , uint32_t index , vector <uint32_t> &commit_size_list)
{
	uint32_t offset = 0;
	uint32_t number_of_commit , number_of_file , number_of_add , number_of_modify , number_of_copy , number_of_delete , commit_size;
	uint8_t file_name_size;
	char file_name[MAX_LENGTH_OF_FILE_NAME] = {0};
	uint8_t MD5[LENGTH_OF_MD5 + 1] = {0};

	for (uint32_t i = 0 ; i < index - 1 ; i++)
		offset += commit_size_list[i];
	fseek(loser_record , offset , SEEK_SET);

	fread(&number_of_commit , sizeof(uint32_t) , 1 , loser_record);
	fread(&number_of_file , sizeof(uint32_t) , 1 , loser_record);
	fread(&number_of_add , sizeof(uint32_t) , 1 , loser_record);
	fread(&number_of_modify , sizeof(uint32_t) , 1 , loser_record);
	fread(&number_of_copy , sizeof(uint32_t) , 1 , loser_record);
	fread(&number_of_delete , sizeof(uint32_t) , 1 , loser_record);
	fread(&commit_size , sizeof(uint32_t) , 1 , loser_record);

	cout << "# commit " << number_of_commit << "\n";

	cout << "[new_file]\n";
	for (uint32_t i = 0 ; i < number_of_add ; i++)
	{
		fread(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fread(file_name , sizeof(char) , file_name_size , loser_record);
		cout << file_name << "\n";
	}

	cout << "[modified]\n";
	for (uint32_t i = 0 ; i < number_of_modify ; i++)
	{
		fread(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fread(file_name , sizeof(char) , file_name_size , loser_record);
		cout << file_name << "\n";
	}

	cout << "[copied]\n";
	for (uint32_t i = 0 ; i < number_of_copy ; i++)
	{
		fread(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fread(file_name , sizeof(char) , file_name_size , loser_record);
		cout << file_name << " => ";
		fread(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fread(file_name , sizeof(char) , file_name_size , loser_record);
		cout << file_name << "\n";
	}

	cout << "[deleted]\n";
	for (uint32_t i = 0 ; i < number_of_delete ; i++)
	{
		fread(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fread(file_name , sizeof(char) , file_name_size , loser_record);
		cout << file_name << "\n";
	}

	cout << "(MD5)\n";
	for (uint32_t i = 0 ; i < number_of_file ; i++)
	{
		fread(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fread(file_name , sizeof(char) , file_name_size , loser_record);
		cout << file_name << " ";
		fread(MD5 , sizeof(uint8_t) , LENGTH_OF_MD5 , loser_record);
		for (uint8_t j = 0 ; j < LENGTH_OF_MD5 ; j++)
			printf("%02x" , MD5[j]);
		cout << "\n";
	}

	return;
}

void write_commit(uint32_t number_of_commit , vector <string> &new_file , vector <string> &modified , vector <pair <string , string> > &copied , vector <string> &deleted , vector <string> &file_list)
{
	FILE *loser_record = fopen(".loser_record" , "ab");

	uint32_t number_of_file , number_of_add , number_of_modify , number_of_copy , number_of_delete , commit_size;
	uint8_t file_name_size;
	char file_name[MAX_LENGTH_OF_FILE_NAME] = {0};
	uint8_t MD5[LENGTH_OF_MD5 + 1] = {0};
	char temp[LENGTH_OF_MD5 + 1] = {0};

	number_of_file = file_list.size();
	number_of_add = new_file.size();
	number_of_modify = modified.size();
	number_of_copy = copied.size();
	number_of_delete = deleted.size();
	commit_size = calculate_commit_size(new_file , modified , copied , deleted , file_list);

	fwrite(&number_of_commit , sizeof(uint32_t) , 1 , loser_record);
	fwrite(&number_of_file , sizeof(uint32_t) , 1 , loser_record);
	fwrite(&number_of_add , sizeof(uint32_t) , 1 , loser_record);
	fwrite(&number_of_modify , sizeof(uint32_t) , 1 , loser_record);
	fwrite(&number_of_copy , sizeof(uint32_t) , 1 , loser_record);
	fwrite(&number_of_delete , sizeof(uint32_t) , 1 , loser_record);
	fwrite(&commit_size , sizeof(uint32_t) , 1 , loser_record);

	for (uint32_t i = 0 ; i < number_of_add ; i++)
	{
		strcpy(file_name , new_file[i].c_str());
		file_name_size = strlen(file_name);
		fwrite(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fwrite(file_name , sizeof(char) , file_name_size , loser_record);
	}

	for (uint32_t i = 0 ; i < number_of_modify ; i++)
	{
		strcpy(file_name , modified[i].c_str());
		file_name_size = strlen(file_name);
		fwrite(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fwrite(file_name , sizeof(char) , file_name_size , loser_record);
	}

	for (uint32_t i = 0 ; i < number_of_copy ; i++)
	{
		strcpy(file_name , copied[i].first.c_str());
		file_name_size = strlen(file_name);
		fwrite(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fwrite(file_name , sizeof(char) , file_name_size , loser_record);

		strcpy(file_name , copied[i].second.c_str());
		file_name_size = strlen(file_name);
		fwrite(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fwrite(file_name , sizeof(char) , file_name_size , loser_record);
	}

	for (uint32_t i = 0 ; i < number_of_delete ; i++)
	{
		strcpy(file_name , deleted[i].c_str());
		file_name_size = strlen(file_name);
		fwrite(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fwrite(file_name , sizeof(char) , file_name_size , loser_record);
	}

	for (uint32_t i = 0 ; i < number_of_file ; i++)
	{
		strcpy(file_name , file_list[i].c_str());
		file_name_size = strlen(file_name);
		fwrite(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fwrite(file_name , sizeof(char) , file_name_size , loser_record);

		getFileMd5(file_name , temp);
		for (uint8_t j = 0 ; j < LENGTH_OF_MD5 ; j++)
			MD5[j] = temp[j];

		fwrite(MD5 , sizeof(uint8_t) , LENGTH_OF_MD5 , loser_record);
	}

	fclose(loser_record);
	return;
}

void classify(vector <string> &file_list , unordered_map <string , string> &previous_file_list , vector <string> &new_file , vector <string> &modified , vector <pair <string , string> > &copied , vector <string> &deleted)
{
	uint32_t number_of_file = file_list.size();

	for (uint32_t i = 0 ; i < number_of_file ; i++)
	{
		string original_file_name;
		char MD5_1[LENGTH_OF_MD5 + 1] = {0} , MD5_2[LENGTH_OF_MD5 + 1] = {0};
		bool find_file_name = false , find_MD5 = false;

		getFileMd5(file_list[i].c_str() , MD5_1);

		for (unordered_map <string , string>::iterator it = previous_file_list.begin() ; !find_file_name && it != previous_file_list.end() ; it++)
		{
			strcpy(MD5_2 , it->second.c_str());

			if (it->first == file_list[i])
			{
				find_file_name = true;
				find_MD5 = same_MD5(MD5_1 , MD5_2);
			}

			if (same_MD5(MD5_1 , MD5_2))
			{
				find_MD5 = true;

				if (original_file_name == "" || it->first < original_file_name)
					original_file_name = it->first;
			}
		}

		if (find_file_name)
		{
			if (!find_MD5)
				modified.push_back(file_list[i]);
		}
		else
		{
			if (!find_MD5)
				new_file.push_back(file_list[i]);
			else
			{
				copied.push_back(make_pair(original_file_name , file_list[i]));
			}
		}
	}

	for (uint32_t i = 0 ; i < number_of_file ; i++)
		previous_file_list.erase(file_list[i]);
	for (unordered_map <string , string>::iterator it = previous_file_list.begin() ; it != previous_file_list.end() ; it++)
		deleted.push_back(it->first);

	return;
}

void status()
{
	vector <string> file_list;
	unordered_map <string , string> previous_file_list;
	uint32_t number_of_file;
	vector <uint32_t> commit_size_list;
	uint32_t number_of_commit;
	vector <string> new_file , modified , deleted;
	vector <pair <string , string> > copied;
	uint32_t number_of_add , number_of_modify , number_of_copy , number_of_delete;

	list_file(file_list);
	number_of_file = file_list.size();

	FILE *loser_record = fopen(".loser_record" , "rb");

	if (!loser_record)
	{
		for (uint32_t i = 0 ; i < number_of_file ; i++)
			new_file.push_back(file_list[i]);
	}
	else
	{
		list_commit_size(loser_record , commit_size_list);
		number_of_commit = commit_size_list.size();
		list_commit(loser_record , number_of_commit , commit_size_list , previous_file_list);
		classify(file_list , previous_file_list , new_file , modified , copied , deleted);

		fclose(loser_record);
	}

	sort(new_file.begin() , new_file.end());
	sort(modified.begin() , modified.end());
	sort(copied.begin() , copied.end() , compare_copied);
	sort(deleted.begin() , deleted.end());

	cout << "[new_file]\n";
	number_of_add = new_file.size();
	for (uint32_t i = 0 ; i < number_of_add ; i++)
		cout << new_file[i] << "\n";

	cout << "[modified]\n";
	number_of_modify = modified.size();
	for (uint32_t i = 0 ; i < number_of_modify ; i++)
		cout << modified[i] << "\n";

	cout << "[copied]\n";
	number_of_copy = copied.size();
	for (uint32_t i = 0 ; i < number_of_copy ; i++)
		cout << copied[i].first << " => " << copied[i].second << "\n";

	cout << "[deleted]\n";
	number_of_delete = deleted.size();
	for (uint32_t i = 0 ; i < number_of_delete ; i++)
		cout << deleted[i] << "\n";

	return;
}

void commit()
{
	vector <string> file_list;
	unordered_map <string , string> previous_file_list;
	uint32_t number_of_file;
	vector <uint32_t> commit_size_list;
	uint32_t number_of_commit;
	vector <string> new_file , modified , deleted;
	vector <pair <string , string> > copied;

	list_file(file_list);
	number_of_file = file_list.size();

	FILE *loser_record = fopen(".loser_record" , "rb");

	if (!loser_record)
	{
		for (uint32_t i = 0 ; i < number_of_file ; i++)
			new_file.push_back(file_list[i]);
	}
	else
	{
		list_commit_size(loser_record , commit_size_list);
		number_of_commit = commit_size_list.size();
		list_commit(loser_record , number_of_commit , commit_size_list , previous_file_list);
		classify(file_list , previous_file_list , new_file , modified , copied , deleted);

		fclose(loser_record);
	}

	sort(new_file.begin() , new_file.end());
	sort(modified.begin() , modified.end());
	sort(copied.begin() , copied.end() , compare_copied);
	sort(deleted.begin() , deleted.end());

	write_commit(number_of_commit + 1 , new_file , modified , copied , deleted , file_list);

	return;
}

void log(uint32_t number_of_log)
{
	FILE *loser_record = fopen(".loser_record" , "rb");

	if (!loser_record)
		return;

	vector <uint32_t> commit_size_list;
	uint32_t number_of_commit;

	list_commit_size(loser_record , commit_size_list);
	number_of_commit = commit_size_list.size();
	number_of_log = min(number_of_log , number_of_commit);

	for (uint32_t index = number_of_commit ; index >= number_of_commit - number_of_log + 1 ; index--)
	{
		print_commit(loser_record , index , commit_size_list);
		if (index != number_of_commit - number_of_log + 1)
			cout << "\n";
	}
	
	fclose(loser_record);
	return;
}
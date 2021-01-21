#ifndef LOSER_H
#define LOSER_H

#include <stdint.h>
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

#define MAX_LENGTH_OF_FILE_NAME 256

bool compare_copied(const pair <string , string> &copied_1 , const pair <string , string> &copied_2);
void list_file(vector <string> &file_list);
void list_commit_size(FILE *loser_record , vector <uint32_t> &commit_size_list);
uint32_t calculate_commit_size(vector <string> &new_file , vector <string> &modified , vector <pair <string , string> > &copied , vector <string> &deleted , vector <string> &file_list);
void list_commit(FILE *loser_record , uint32_t index , vector <uint32_t> &commit_size_list , unordered_map <string , string> &file_list);
void print_commit(FILE *loser_record , uint32_t index , vector <uint32_t> &commit_size_list);
void write_commit(uint32_t number_of_commit , vector <string> &new_file , vector <string> &modified , vector <pair <string , string> > &copied , vector <string> &deleted , vector <string> &file_list);
void classify(vector <string> &file_list , unordered_map <string , string> &previous_file_list , vector <string> &new_file , vector <string> &modified , vector <pair <string , string> > &copied , vector <string> &deleted);
void status();
void commit();
void log(uint32_t number_of_log);

#endif

#include <iostream>
#include <b15f/b15f.h>
#include <vector>
#include <array>

using namespace std;

//Process
void assignPC(B15F&);
void sending(B15F&, vector<vector<uint8_t>>);
vector<vector<uint8_t>> receiving(B15F&);

//Helper
vector<uint8_t> dissectBinary(uint8_t);
vector<uint8_t> createBinaryVector(uint8_t);
vector<vector<uint8_t>> createSentenceVector(const string);
uint8_t sendConverting(uint8_t);
uint8_t reveiceConverting(uint8_t);
int countOnes(uint8_t);
bool isEven(int);
void setParity(vector<vector<uint8_t>>&);

//Debug
void leitungstest(B15F&);
void clearLeitung(B15F&);
void print2dVector(vector<vector<uint8_t>>);
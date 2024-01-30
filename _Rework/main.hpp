#include <iostream>
#include <unistd.h>
#include <b15f/b15f.h>
#include <vector>
#include <array>
#include <chrono>
#include <ctime>

using namespace std;

//Process
void assignPC(B15F&);
void sending(B15F&, vector<vector<uint8_t>>);
vector<vector<uint8_t>> receiving(B15F&);

//Parity
vector<vector<uint8_t>> setParity(vector<vector<uint8_t>>&);
bool checkParity(vector<uint8_t>);
void removeParity(vector<uint8_t>&);
uint8_t mergeBitsets(vector<uint8_t>);
bool sendRequest(B15F &drv);

//Helper
uint8_t sendConverting(uint8_t);
uint8_t reveiceConverting(uint8_t);
vector<uint8_t> createBinaryVector(uint8_t);
vector<vector<uint8_t>> createSentenceVector(const string);
int countOnes(uint8_t);
bool isEven(int);

//Debug
void leitungstest(B15F&);
void printSentenceVector(vector<vector<uint8_t>>);
void printBinaryVectorToChar(vector<uint8_t>);
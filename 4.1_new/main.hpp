#include <iostream>
#include <b15f/b15f.h>
#include <vector>
#include <array>

using namespace std;

void sendEscape(B15F&);
void sendChar(char, B15F&);
bool checkIfStartSign(B15F&);
bool checkIfStopSign(B15F&);
void revieceChar(B15F&);
char binaryToChar(const string);
void sendStartSign(B15F&);
void sendStopSign(B15F&);
void assignPC(B15F&);
void sendChar(B15F&);

uint8_t sendConverting(uint8_t);
uint8_t reveiceConverting(uint8_t);
void sending(B15F&, char);
uint8_t addTactToPackage(uint8_t);
vector<uint8_t> dissectBinary(uint8_t);
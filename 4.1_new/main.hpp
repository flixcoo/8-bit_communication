#include <iostream>
#include <b15f/b15f.h>
#include <vector>

using namespace std;

void sendEscape(B15F&);
void sendChar(char, B15F&);
bool checkIfStartSign(B15F&);
bool checkIfStopSign(B15F&);
void revieceChar(B15F&);
char binaryToChar(const string);
void sendStartSign(B15F&);
void sendStopSign(B15F&);
void assignPC(B15F);
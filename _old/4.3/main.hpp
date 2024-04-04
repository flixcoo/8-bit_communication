#include <iostream>
#include <fstream>
#include <b15f/b15f.h>
#include <vector>
#include <string>

using namespace std;

//Sending Functions
void sendEscape(B15F&);
void sendLength(int, B15F&);
void sendSentence(vector<char>, B15F&);

//Receiveing Functions
string receiveMode(B15F&);
bool checkEscape(B15F&);
int receiveLength(B15F&);
string receiveSentence(B15F&, const int);

//Helper Functions
vector<char> dissectString(const string);
char binaryToChar(const string);
int binaryToDecimal(const string);
bool checkParity(const string);
string setParity(string &binary);
void printSentenceToTxt(const string, ofstream&);

#include <iostream>
#include <b15f/b15f.h>

using namespace std;

void revieceChar(B15F&);
void sendChar(char, B15F&);
void sendEscape(B15F&);
string setStringtoASCII(string);
int binaryToDecimal(string);



int main() {
	B15F& drv = B15F::getInstance();
	drv.setRegister(&DDRA, 0x07);
	
	while(1)
	{
		int decision;
		cout << "Was möchten Sie tun?\n[0] Empfangen\n[1] Senden" << endl;
		cin >> decision;

		if((!cin.fail()) && (decision < 2 && decision > -1))
		{
			cout << "d: " << decision << endl;
			if(decision)
			{
				cout << "Bitte geben Sie einen Buchstaben zum Senden ein:" << endl;
				char c;
				cin >> c;
				sendEscape(drv);
				sendChar(c,drv);
			}
			else
			{
				revieceChar(drv);
			}
		}
		//skipping wrong inputs 
		cin.clear();
		cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
}

void sendEscape(B15F& drv)
{
	// ESC: 101-010-101
	drv.setRegister(&DDRA, 0x07);
	drv.setRegister(&PORTA, 101);
	drv.delay_ms(500);
	drv.setRegister(&PORTA, 010);
	drv.delay_ms(500);
	drv.setRegister(&PORTA, 101);
	drv.delay_ms(500);
}

void revieceChar(B15F& drv)
{
	
	//checking if escape was send
	bool active = false;
	
	while(!active)
	{
		int input = (int)drv.getRegister(&PINA);
		if(input == 101)
		{
			drv.delay_ms(500);
			input = (int)drv.getRegister(&PINA);
			if(input == 010)
			{
				drv.delay_ms(500);
				input = (int)drv.getRegister(&PINA);
				if(input == 101)
				{
					active = true;	
				}
			}
		}	
	}
	
	drv.setRegister(&DDRA, 0x00);
	string binary = "";
	for(int i = 0; i < 3; i++)
	{
		cout << "[Empfangen]: " <<((int) drv.getRegister(&PINA)) << endl;
		binary += drv.getRegister(&PINA);
		drv.delay_ms(500);
	}	
}

void sendChar(char c, B15F& drv)
{
	drv.setRegister(&DDRA, 0x07);
	//9-Bit Zahl
	string binary = std::bitset<9>((int)c).to_string();
	
	for(long unsigned int i = 0; i < binary.length();i += 3)
	{
		cout << "substring: " << binary.substr(i,3) << endl; //Debug
		int bin = stoi(binary.substr(i,3));
		drv.setRegister(&PORTA, bin);
		drv.delay_ms(500);
	}
}

string setStringtoASCII(string str)
{
    int size = int(str.size());
 
    // If given string is not a valid string
    if (size % 8 != 0) {
        return "Not Possible!";
    }
 
    // To store final answer
    string asciiStr = "";
 
    // Loop to iterate through string
    for (int i = 0; i < size; i += 8) {
        int decimal_value = binaryToDecimal((str.substr(i, 8)));
 
        // Apprend the ASCII character equivalent to current value
        asciiStr += char(decimal_value);
    }
    return asciiStr;
}

int binaryToDecimal(string binary)
{
    // Stores the decimal value
    int dec_value = 0;
    // Initializing base value to 1
    int base = 1;
 
    int len = binary.length();
    for (int i = len - 1; i >= 0; i--) {
 
        // If the current bit is 1
        if (binary[i] == '1')
            dec_value += base;
        base = base * 2;
    }
    // Return answer
    return dec_value;
}



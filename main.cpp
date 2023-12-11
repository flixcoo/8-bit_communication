#include <iostream>
#include <b15f/b15f.h>
#include <vector>

using namespace std;

void revieceChar(B15F&);
void sendChar(char, B15F&);
void sendEscape(B15F&);
string setStringtoASCII(string);
int binaryToDecimal(string);



int main() {
	B15F& drv = B15F::getInstance();
	cout << endl << "-------[V-1.1]-------" << endl << endl;
	
	while(1)
	{
		int decision;
		cout << "Was möchten Sie tun?\n[0] Empfangen\n[1] Senden" << endl;
		cin >> decision;

		if((!cin.fail()) && (decision < 2 && decision > -1))
		{
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
				/*
				while(1)
					cout << "[Recieved]: " << (int)drv.getRegister(&PINA) << endl;
				*/
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
	cout << "[System]: ESC wird gesendet..." << endl;
	drv.setRegister(&PORTA, 0b101); //5
	drv.delay_ms(500);
	drv.setRegister(&PORTA, 0b010); //2
	drv.delay_ms(500);
	drv.setRegister(&PORTA, 0b101); //5
	cout << "[System]: ESC gesendet" << endl;
	drv.delay_ms(500);
}

void revieceChar(B15F& drv)
{
	drv.setRegister(&DDRA, 0x00);
	//checking if escape was send
	bool active = false;
	int input;
	drv.setRegister(&DDRA, 0x00);
	
	while(!active)
	{
		
		drv.delay_ms(500);
		input = (int)drv.getRegister(&PINA);
		//cout << "[System]: " << input << "empfangen" << endl; 
		if(input == 5)
		{
			//cout << "[Debug]: input 5" << endl;
			drv.delay_ms(500);
			input = (int)drv.getRegister(&PINA);
			if(input == 2)
			{
			     	//cout << "[Debug]: input 2" << endl;
				drv.delay_ms(500);
				input = (int)drv.getRegister(&PINA);
				if(input == 5)
				{
					//cout << "[Debug]: input 5 erneut" << endl;
					active = true;	
				}
				else
					cout << "[System]: Kein ESC bekommen" << endl;	
			}
			else
				cout << "[System]: Kein ESC bekommen" << endl;	
		}
		else
			cout << "[System]: Kein ESC bekommen" << endl;	
	}
	cout << "[System]: ESC empfangen" << endl;
	
	vector<int> binary;
	for(int i = 0; i < 3; i++)
	{
		drv.delay_ms(500);
		binary.push_back((int)drv.getRegister(&PINA));
	}	
	
	string s = "";
	for(int i = 0; i < 3; i++)
		s += std::bitset<3>(binary.at(i)).to_string();
	
	cout << "[System]: " << s << " empfangen" << endl;
	
}

void sendChar(char c, B15F& drv)
{
	drv.setRegister(&DDRA, 0x07);
	string binary;
	
	if(isdigit(c))
	{
		int val = c - '0' ;
		binary = bitset<9>(val).to_string(); //9-Bit Zahl, da 9 % 3 == 0
		cout << "Wert: " << val << endl << "Binary: " << binary << endl; //Debug
	}
	else
	{
		binary = bitset<9>((int)c).to_string();
		cout << "Wert: " << c << endl << "Binary: " << binary << endl; //Debug
	}
	
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



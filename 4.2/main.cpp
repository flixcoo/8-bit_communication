#include <iostream>
#include <b15f/b15f.h>
#include <vector>

using namespace std;

vector<char> dissectString(const string&);
void sendChar(const char, B15F &);
void revieceChar(B15F &);
char binToChar(const string);
bool checkEscape(B15F &);
void sendEscape(B15F &);

bool debug = true;

int main()
{
	B15F &drv = B15F::getInstance();
	
	string sentence;
	vector<char> charVector;

	if (std::getline(cin, sentence))
	{
		if (sentence == "")
		{
			cout << "[System]: Empf„nger" << endl;
			while (1)
				revieceChar(drv);
		}
		else
		{
			charVector = dissectString(sentence);
			sendEscape(drv);
			int size = charVector.size();
			if(debug){cout << "size: " << size << endl;}
			sendChar(size, drv); //Groesse rueberschicken
			for (char c : charVector)
				sendChar(c, drv);
		}
	}
	else
		cout << "[System]: Error" << endl;
}

vector<char> dissectString(const string& sentence)
{
	vector<char> vector;
	if (sentence.empty())
	{
		cout << "falsche Eingabe" << endl;
	}
	vector.assign(sentence.begin(), sentence.end());
	
	if(debug){cout << "size-dissectString: " << vector.size() << endl;}
	return vector;
}

void sendChar(const char c, B15F &drv)
{
	drv.setRegister(&DDRA, 0x07);
	string binary = bitset<9>((int)c).to_string();
	cout << "Wert: " << c << endl
		 << "Binary: " << binary << endl; // Debug

	for (long unsigned int i = 0; i < binary.length(); i += 3)
	{
		drv.setRegister(&PORTA, stoi(binary.substr(i, 3)));
		drv.delay_ms(500);
	}
}

void revieceChar(B15F &drv)
{
	drv.setRegister(&DDRA, 0x00);
	// checking if escape was send
	bool active = false;
	while (!active)
	{	
		active = checkEscape(drv);
		if(debug){cout << "active " << active << endl;}
	}
	if(debug){cout << "Vor Amount: " << (int)drv.getRegister(&PINA) << endl;}
	drv.delay_ms(500);	
	int amount = (int)drv.getRegister(&PINA);
	// @todo
	// Amount war beim Empfaenger-PC 5 statt 4

	//drv.delay_ms(500);
	if(debug){cout << "amount: " << amount << endl;}

	vector<int> binary;
	if(debug){cout << "hier1" << endl;}
	for (int i = 0; i < amount; i++)
	{
		for (int i = 0; i < 3; i++)
		{
			drv.delay_ms(500);
			if(debug){cout << "Register: " << (int)drv.getRegister(&PINA) << endl;}
			binary.push_back((int)drv.getRegister(&PINA));
		}

		string binaryString = "";
		for (int i = 0; i < 3; i++)
			binaryString += std::bitset<3>(binary.at(i)).to_string();
		string s = "abc";
		cout << "[System]: " << binaryString << " empfangen" << endl;
		cout << "[System]: " << binToChar(binaryString) << " (ASCII-Umwandlung)" << endl;
	}
}

char binToChar(const string s)
{
	char c = 0;
	for (int i = 0; i < 9; i++)
	{
		if ('1' == s.at(i))
		{
			c += (int)pow(2, 8 - i);
		}
	}
	return c;
}

bool checkEscape(B15F &drv)
{
	drv.delay_ms(250);
	int input = (int)drv.getRegister(&PINA);
	if (input == 5)
	{	
		drv.delay_ms(500);
		input = (int)drv.getRegister(&PINA);
		if (input == 2)
		{
			drv.delay_ms(500);
			input = (int)drv.getRegister(&PINA);
			if (input == 5)
			{
				drv.delay_ms(500);
				cout << "[System]: ESC empfangen" << endl;
				return true;
			}
		}
	}
	cout << "[System]: Kein ESC bekommen" << endl;
	return false;
}

void sendEscape(B15F &drv)
{
    drv.setRegister(&DDRA, 0x07);
    cout << "[System]: ESC wird gesendet..." << endl;
    // ESC: 101-010-101 or 5-2-5
    drv.setRegister(&PORTA, 0b101);
    drv.delay_ms(500);
    drv.setRegister(&PORTA, 0b010);
    drv.delay_ms(500);
    drv.setRegister(&PORTA, 0b101);
    cout << "[System]: ESC gesendet" << endl;
    drv.delay_ms(500);
}
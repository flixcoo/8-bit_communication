#include <iostream>
#include <b15f/b15f.h>
#include <vector>
#include <string>

#include "main.hpp"

using namespace std;

int main()
{
	B15F &drv = B15F::getInstance();
	bool running = true;

	while (running)
	{
		int decision;
		cout << endl
			 << "Was möchten Sie tun?\n[0] Empfangen\n[1] Senden\n[2] Exit" << endl;
		cin >> decision;

		if ((!cin.fail()) && (decision < 3 && decision > -1))
		{
			if (!decision)
				receiveMode(drv);
			if (decision)
			{
				if (decision == 2)
					running = false;
				else
				{
					string sentence;
					cout << "Bitte geben Sie einen Satz zum Senden ein:" << endl;
					cin.ignore();
					getline(cin, sentence);
					cout << "[System]: Erkannter Satz = \"" << sentence << "\"" << endl;

					vector<char> charVector = dissectString(sentence);
					sendEscape(drv);
					sendSentence(charVector, drv);
				}
			}
		}
		else
		{
			// clearing cin buffer
			cin.clear();
			cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}
	cout << endl
		 << "[System]: Programm beendet" << endl
		 << endl;
}

// main for piping
/* int main()
{
	string sentence;
	if (std::getline(cin, sentence))
	{
		if (sentence == "")
		{
			cout << "[System]: Rechner ist empfänger" << endl;
			receiveMode(drv);
		}
		else
		{
			cout << "sentence: " << sentence << endl;
			vector<char> charVector = dissectString(sentence);
			sendEscape(drv);
			sendSentence(charVector, drv);
		}
	}
} */

void sendEscape(B15F &drv)
{
	drv.setRegister(&DDRA, 0x07);
	cout << "[sendEscape]: ESC wird gesendet..." << endl;
	drv.setRegister(&PORTA, 0b101);
	drv.delay_ms(500);
	drv.setRegister(&PORTA, 0b010);
	drv.delay_ms(500);
	drv.setRegister(&PORTA, 0b101);
	cout << "[sendEscape]: ESC gesendet" << endl;
	drv.delay_ms(500);
}

void sendLength(const int length, B15F &drv)
{
	string lengthBinary = bitset<9>(length).to_string();
	for (long unsigned int i = 0; i < lengthBinary.length(); i += 3)
	{
		drv.setRegister(&PORTA, stoi(lengthBinary.substr(i, 3)));
		drv.delay_ms(500);
	}
}

void sendSentence(const vector<char> charVector, B15F &drv)
{
	int length = charVector.size();
	sendLength(length, drv);
	for (char character : charVector)
	{
		string binary = bitset<9>((int)character).to_string();
		for (long unsigned int i = 0; i < binary.length(); i += 3)
		{
			drv.setRegister(&PORTA, stoi(binary.substr(i, 3)));
			drv.delay_ms(500);
		}
	}
}

void receiveMode(B15F &drv)
{
	drv.setRegister(&DDRA, 0x00);
	bool active = false;
	while (!active)
	{
		active = checkEscape(drv);
	}
	int sentenceLength = receiveLength(drv);
	string sentence = receiveSentence(drv, sentenceLength);
	cout << "[recieve]: Empfangener Satz = " << sentence << endl;
}

bool checkEscape(B15F &drv)
{
	drv.delay_ms(500);
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
				cout << "[checkEscape]: ESC empfangen" << endl;
				return true;
			}
		}
	}
	cout << "[checkEscape]: Kein ESC bekommen" << endl;
	return false;
}

int receiveLength(B15F &drv)
{
	vector<int> sentenceLengthVector;
	for (int i = 0; i < 3; i++) // Gleichung damit das letzte Zeichen mit gezählt wird
	{
		drv.delay_ms(500);
		sentenceLengthVector.push_back((int)drv.getRegister(&PINA));
	}

	string sentenceLengthInBinary = "";
	for (long unsigned int i = 0; i < sentenceLengthVector.size(); i++)
		sentenceLengthInBinary += bitset<3>(sentenceLengthVector.at(i)).to_string();

	cout << "[receiveLength]: Empfangene Länge (Binär) = " << sentenceLengthInBinary
		 << "\n[receiveLength]: Empfangene Länge (Dezimal) = " << binaryToDecimal(sentenceLengthInBinary);
	return binaryToDecimal(sentenceLengthInBinary);
}

string receiveSentence(B15F &drv, const int sentenceLength)
{
	vector<int> receivedNumsVector;

	for (int i = 0; i < sentenceLength * 3; i++) // sentenceLength * 3 = Anzahl an Zeichen die empfangen werden
	{
		cout << "[receiveSentence]: i = " << i << endl;
		cout << "[receiveSentence]: sentenceLength = " << sentenceLength << endl;
		cout << "[receiveSentence]: " << i << " < " << sentenceLength << endl;
		drv.delay_ms(500);
		receivedNumsVector.push_back((int)drv.getRegister(&PINA));
		cout << "[receiveSentence]: Empfangene Zahl = " << receivedNumsVector.at(i) << endl;
	}

	string sentence;
	// for-Schleife für die 3er Pakete
	for (unsigned long int i = 0; i < receivedNumsVector.size(); i += 3)
	{
		string numAsBinary = "";
		for (unsigned long int j = 0; j < 3; j++)
		{ // for-Schleife für die einzelnen 3 Teile jedes  Zeichens
			numAsBinary += bitset<3>(receivedNumsVector.at(i + j)).to_string();
			cout << "[receiveSentence]: " << numAsBinary << " empfangen (binary)" << endl;
		}
		sentence += binaryToChar(numAsBinary);
	}
	return sentence;
}

vector<char> dissectString(const string sentence)
{
	return vector<char>(sentence.begin(), sentence.end());
}

char binaryToChar(const string binary)
{
	char c = 0;
	for (long unsigned int i = 0; i < binary.length(); i++)
		if ('1' == binary.at(i))
			c += (int)pow(2, 8 - i);
	return c;
}

int binaryToDecimal(const string binary)
{
	int decimal = 0;
	for (long unsigned int i = 0; i < binary.length(); i++)
		if ('1' == binary.at(i))
			decimal += pow(2, 8 - i);
	return decimal;
}

// Bei einer geraden Anzahl an '1' wird das MSB (9. Bit) == 0,
// bei einer ungeraden Anzahl wird MSB == 1
bool checkForParity(const string binary)
{
	int count = 0;
	for (long unsigned int i = 1; i < binary.length(); i++)
		if (binary.at(i) == 1)
			count++;
	return (count % 2 == 0) == (binary.at(0) == '0');
}
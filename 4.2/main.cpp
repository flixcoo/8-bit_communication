#include "main.hpp"

int main()
{
	B15F &drv = B15F::getInstance();
	ofstream out("empfangeneSaetze.txt");
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
				printSentenceToTxt(receiveMode(drv), out);
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
					cout << "[System]: Erkannte Eingabe = \"" << sentence << "\"" << endl;

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
	out.close();
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
			receiveMode(drv);
		else
		{
			cout << "[System]: Erkannte Eingabe = " << sentence << endl;
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
	drv.setRegister(&PORTA, 0b101); //Binary 5
	drv.delay_ms(500);
	drv.setRegister(&PORTA, 0b010); //Binary 2
	drv.delay_ms(500);
	drv.setRegister(&PORTA, 0b101); //Binary 5
	cout << "[sendEscape]: ESC gesendet" << endl;
	drv.delay_ms(500);
}

void sendLength(const int length, B15F &drv)
{
	cout << "[sendLength]: Länge wird gesendet..." << endl;
	string lengthBinary = bitset<9>(length).to_string();
	for (long unsigned int i = 0; i < lengthBinary.length(); i += 3)
	{
		drv.setRegister(&PORTA, stoi(lengthBinary.substr(i, 3)));
		drv.delay_ms(500);
	}
	cout << "[sendLength]: Länge gesendet!" << endl;
}

void sendSentence(const vector<char> charVector, B15F &drv)
{
	cout << "[sendSentence]: Satz wird gesendet..." << endl;
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
		cout << "[sendSentence]: Character gesendet" << endl;
	}
}

string receiveMode(B15F &drv)
{
	cout << "[receiveMode]: Modus - Empfangen" << endl;
	drv.setRegister(&DDRA, 0x00);
	bool active = false;
	while (!active)
	{
		active = checkEscape(drv);
	}
	int sentenceLength = receiveLength(drv);
	string sentence = receiveSentence(drv, sentenceLength);
	cout << "[receiveMode]: Empfangener Satz = " << sentence << endl;
	return sentence;
}

bool checkEscape(B15F &drv)
{
	drv.delay_ms(100);
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
	cout << "[receiveLength]: Länge wird empfangen..." << endl;
	for (int i = 0; i < 3; i++) // Gleichung damit das letzte Zeichen mit gezählt wird
	{
		drv.delay_ms(500);
		sentenceLengthVector.push_back((int)drv.getRegister(&PINA));
	}

	string sentenceLengthInBinary = "";
	for (long unsigned int i = 0; i < sentenceLengthVector.size(); i++)
		sentenceLengthInBinary += bitset<3>(sentenceLengthVector.at(i)).to_string();

	cout << "[receiveLength]: Empfangene Länge (Binär) = " << sentenceLengthInBinary << endl;
	return binaryToDecimal(sentenceLengthInBinary);
}

string receiveSentence(B15F &drv, const int sentenceLength)
{
	vector<int> receivedNumsVector;
	cout << "[receiveSentence]: Satz wird empfangen..." << endl;
	for (int i = 0; i < sentenceLength * 3; i++) // sentenceLength * 3 = Anzahl an Zeichen die empfangen werden
	{
		drv.delay_ms(500);
		receivedNumsVector.push_back((int)drv.getRegister(&PINA));
		cout << "[receiveSentence]: Zahl empfangen [" << receivedNumsVector.at(i) << "]" << endl;
		if(i % 3 == 0 && i != 0)
			cout << "[receiveSentence]: Character vollständig empfangen" << endl;
	}

	string sentence;
	// for-Schleife für die 3er Pakete
	for (unsigned long int i = 0; i < receivedNumsVector.size(); i += 3)
	{
		string numAsBinary = "";
		for (unsigned long int j = 0; j < 3; j++) // for-Schleife für die einzelnen 3 Teile jedes  Zeichens
			numAsBinary += bitset<3>(receivedNumsVector.at(i + j)).to_string();
		sentence += binaryToChar(numAsBinary);
	}
	cout << "[receiveSentence]: Satz vollständig empfangen" << endl;
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

void printSentenceToTxt(const string sentence, ofstream &out)
{
	out << sentence;
	out << "\n";
	cout << "[printSentenceToTxt]: Satz wurde in empfangeneSaetze.txt geschrieben" << endl;
}
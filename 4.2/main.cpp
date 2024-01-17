#include <iostream>
#include <b15f/b15f.h>
#include <vector>
#include <string>

using namespace std;

void dissectString(const string&, vector<char>&);
void sendEscape(B15F &);
bool checkEscape(B15F &);
void sendSentence(vector<char>& charVector, B15F &drv);
string receiveSentence(B15F &drv, int sentenceLength);
string binToString(const vector<int> &binary);
void sendLength(int length, B15F &drv);
int receiveLength(B15F &drv);
void receive(B15F &drv);
char binaryToChar(string binary);
bool checkForParity(string binary);
int binaryToDecimal(string binary);

int main() {
	B15F &drv = B15F::getInstance();
	
	bool running = true;
    	while (running) {
        	int decision;
        	cout << endl
             	<< "Was möchten Sie tun?\n[0] Empfangen\n[1] Senden\n[2] Exit" << endl;
        	cin >> decision;

	       	if ((!cin.fail()) && (decision < 3 && decision > -1)) {
				if (!decision)
					receive(drv);
				if (decision) {
				    if (decision == 2)
				        running = false;
				    else {
				        cout << "Bitte geben Sie einen Satz zum Senden ein:" << endl;
				        string sentence;
						vector<char> charVector;
						cin.ignore();
						getline(cin, sentence);
						cout << "sentence: " << sentence << endl;
				        dissectString(sentence, charVector);
				        sendEscape(drv);
				        sendSentence(charVector, drv);
				    }
				}
			else{
				// skipping wrong inputs
       			cin.clear();
        		cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			}
		}
		cout << "test5" << endl;
    }
    cout << endl << "[System]: Programm beendet" << endl << endl;
}

void dissectString(const string& sentence, vector<char>& charVector) {
	if(sentence.empty()) {
		cout << "[System]: Variable 'Sentence' ist leer" << endl;
	}
	charVector.assign(sentence.begin(), sentence.end());
}

void sendEscape(B15F &drv)
{
    
    drv.setRegister(&DDRA, 0x07);
    cout << "[sendEscape]: ESC wird gesendet..." << endl;
    // ESC: 101-010-101 or 5-2-5
    drv.setRegister(&PORTA, 0b101);
    drv.delay_ms(500);
    drv.setRegister(&PORTA, 0b010);
    drv.delay_ms(500);
    drv.setRegister(&PORTA, 0b101);
    cout << "[sendEscape]: ESC gesendet" << endl;
    drv.delay_ms(500);
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

void sendSentence(vector<char>& charVector, B15F &drv) {
	int length = charVector.size();
	sendLength(length, drv);
	for(char character : charVector) {
		string binary = bitset<9>((int)character).to_string();
		for(long unsigned int i = 0; i < binary.length(); i += 3) {
			drv.setRegister(&PORTA, stoi(binary.substr(i, 3)));
			cout << "[sendSentence]: Sending " << binary.substr(i, 3) << endl;
			drv.delay_ms(500);
		}
	}
}

void receive(B15F &drv) {
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

string receiveSentence(B15F &drv, int sentenceLength) {
    vector<int> receivedNumsVector;

    for (int i = 0; i < sentenceLength * 3; i++) //sentenceLength * 3 = Anzahl an Zeichen die empfangen werden
    {
        cout << "[receiveSentence]: i = " << i << endl;
		cout << "[receiveSentence]: sentenceLength = " << sentenceLength << endl;
		cout << "[receiveSentence]: " << i << " < "<< sentenceLength << endl;
		drv.delay_ms(500);
        receivedNumsVector.push_back((int)drv.getRegister(&PINA)); 
		cout << "[receiveSentence]: Empfangene Zahl = " << receivedNumsVector.at(i) << endl;
    }

	//debug
	//printing received numbers
	cout << "[receiveSentence]: Empfangene Nums" << endl << endl;
   	for(long unsigned int i = 0; i < receivedNumsVector.size(); i++){
		cout << "[" << receivedNumsVector.at(i) << "] ";
		if(i+1 % 3 == 0)
			cout << endl;
	}
	cout << endl << endl;
   		
	string Sentence;
	//for-Schleife für die 3er Pakete
    for (unsigned long int i = 0; i < receivedNumsVector.size(); i += 3) { 
		string numAsBinary = "";
		for (unsigned long int j = 0; j < 3 ; j++) {//for-Schleife für die einzelnen 3 Teile jedes  Zeichens
		    numAsBinary += bitset<3>(receivedNumsVector.at(i+j)).to_string();
		cout << "[receiveSentence]: " << numAsBinary << " empfangen (binary)" << endl;
		
		}
		Sentence += binaryToChar(numAsBinary);
	}
	return Sentence;
}

int receiveLength(B15F &drv) {

	vector<int> sentenceLengthVector;
   for (int i = 0; i < 3 ; i++) //Gleichung damit das letzte Zeichen mit gezählt wird
    {
		drv.delay_ms(500);
		cout << "[receiveLength]: receiving now" << endl;
        sentenceLengthVector.push_back((int)drv.getRegister(&PINA)); 
		cout << "[receiveLength]: i = " << i << endl;
    }
	
	//Debug
	cout << "[Debug]: length-vector: ";
	for (long unsigned int i = 0; i < sentenceLengthVector.size(); i++)
		cout << "[" << sentenceLengthVector.at(i) <<"] ";
	cout << endl;
	//Debug 
	
	string sentenceLengthInBinary = "";
	for (long unsigned int i = 0; i < sentenceLengthVector.size(); i++) {
		cout << "[receiveLength]: i = " << i << endl;
		cout << "[receiveLength]: sentenceLengthVector.at(i) = " << sentenceLengthVector.at(i) << endl;
		sentenceLengthInBinary += bitset<3>(sentenceLengthVector.at(i)).to_string();
		cout << "[receiveLength]: sentenceLengthInBinary = " << sentenceLengthInBinary << endl;
	}
	cout << "[receiveLength]: Empfangene Länge (Binär) = " << sentenceLengthInBinary << endl;
	cout << "[receiveLength]: Empfangene Länge (Dezimal) = " << binaryToDecimal(sentenceLengthInBinary) << endl;
	return binaryToDecimal(sentenceLengthInBinary);
}

void sendLength(int length, B15F &drv) {
	string binary = bitset<9>(length).to_string();
	cout << "[sendLength]: Länge (Dezimal) = " << length << "\n[sendLength]: Länge (Binär) = " << binary << endl;
		
	for(long unsigned int i = 0; i < binary.length(); i += 3) {
		cout << "[sendLength]: Sending = " << binary.substr(i, 3) << endl;
		drv.setRegister(&PORTA, stoi(binary.substr(i, 3)));
		drv.delay_ms(500);
	}
}

char binaryToChar(string binary)
{
    char c = 0;
    for (int i = 0; i < binary.length(); i++)
        if ('1' == binary.at(i))
            c += (int)pow(2, 8 - i);
    return c;
}

int binaryToDecimal(string binary)
{
    int decimal = 0;
    for (int i = 0; i < binary.length(); i++)
        if ('1' == binary.at(i))
            decimal += pow(2, 8 - i);
    return decimal;
}

//Bei einer geraden Anzahl an '1' wird das MSB (9. Bit) == 0,
//bei einer ungeraden Anzahl wird MSB == 1
bool checkForParity(string binary)
{
	int count = 0;
	for(int i = 1; i < binary.length(); i++)
		if(binary.at(i) == 1)
			count++;
	return (count % 2 == 0) == (binary.at(0) == '0');
}
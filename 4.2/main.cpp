#include <iostream>
#include <b15f/b15f.h>
#include <vector>
#include <string>

using namespace std;

void dissectString(const string&, vector<char>&);
void sendEscape(B15F &);
bool checkEscape(B15F &);
void sendSentence(vector<char>& charVector, B15F &drv);
void receiveChar(B15F &drv, int length_number);
string binToString(const vector<int> &binary);
char binToChar(const string &in );
void sendLength(int length, B15F &drv);
int receiveLength(B15F &drv);
void receive(B15F &drv);
char binToChar(string &bitfolge);

string s = "";

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
                    cin >> sentence;
                    //cin.ignore(); // Clear newline character from previous input
                    //getline(cin, sentence);
                    vector<char> charVector;
                    dissectString(sentence, charVector); 
                    sendEscape(drv);
                    sendSentence(charVector, drv);
                }
            }
        }
        // skipping wrong inputs
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    cout << endl << "[System]: Programm beendet" << endl << endl;
}

void dissectString(const string& sentence, vector<char>& charVector) {
	if(sentence.empty()) {
		cout << "falsche Eingabe" << endl;
	}
	charVector.assign(sentence.begin(), sentence.end());
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
                cout << "[System]: ESC empfangen" << endl;
                return true;
            }
        }
    }
    cout << "[System]: Kein ESC bekommen" << endl;
    return false;
}



void sendSentence(vector<char>& charVector, B15F &drv) {
	int length = charVector.size();
	sendLength(length, drv);
	for(char character : charVector) {
		string binary = bitset<9>((int)character).to_string();
		cout << "Wert: " << character << endl << "Binary: " << binary << endl;
		
		for(long unsigned int i = 0; i < binary.length(); i += 3) {
			drv.setRegister(&PORTA, stoi(binary.substr(i, 3)));
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
    int length_number = receiveLength(drv);
    receiveChar(drv, length_number);
    cout << "Der Satz ist: " << endl << s << endl;
}

void receiveChar(B15F &drv, int length) {
    vector<int> binary;
    for (int i = 0; i < (length + 1) * 3; i++) //Gleichung damit das letzte Zeichen mit gezählt wird
    {
        drv.delay_ms(500);
        binary.push_back((int)drv.getRegister(&PINA)); 
    }
   	for(int zahl : binary) {
   		cout << zahl << endl;
   	}
    cout << "Bin hier";
    for(int i = 0; i < length; i += 3) {
    	string bitfolge = "";
    	for(int j = 0; j < 3; j++) {
    		bitfolge += bitset<3>(binary.at(i + j)).to_string();
    		cout << "[System]: " << s << " empfangen" << endl;
    		s += binToChar(bitfolge); 
    	}
    }
    //for(int i = 0; i < 3; i++) {
    	//bitfolge += bitset<3>(binary.at(i)).to_string();
    //}

    //string sentence = binToString(binary);
    //cout << "[System]: " << sentence << " empfangen" << endl;
}

int receiveLength(B15F &drv) {
	int length_number = (int)drv.getRegister(&PINA);
	return length_number;
}

void sendLength(int length, B15F &drv) {
	string binary = bitset<9>(length).to_string();
	cout << "Wert: " << length << endl << "Binary: " << binary << endl;
		
	for(long unsigned int i = 0; i < binary.length(); i += 3) {
		drv.setRegister(&PORTA, stoi(binary.substr(i, 3)));
		drv.delay_ms(500);
	}
}

char binToChar(string &bitfolge) {
	cout << "Bin hier in Char";
    char c = 0;
    for (int i = 0; i < 9; i++)
    {
        if ('1' == bitfolge.at(i))
        {
            c += (int)pow(2, 8 - i);
        }
    }
    return c;
}








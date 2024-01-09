#include <iostream>
#include <b15f/b15f.h>
#include <vector>

using namespace std;

vector<char> dissectString(const string &);
void sendChar(const char, B15F &);
void receiveChar(B15F &);
char binToChar(const string);
bool checkEscape(B15F &);
void sendEscape(B15F &);

bool debug = true;

int main()
{
	B15F &drv = B15F::getInstance();

	string sentence = "test";
	vector<char> charVector;
	bool running = true;
	/*if (std::getline(cin, sentence))
	{
		if (sentence == "")
		{
			cout << "[System]: Recipient" << endl;
			while (1)
				receiveChar(drv);
		}
		else
		{
			charVector = dissectString(sentence);
			sendEscape(drv);
			int size = charVector.size();
			if (debug){cout << "charVector.size: " << size << endl;} // Debug
			/*sendChar(size, drv); // Groesse rueberschicken
			for (char c : charVector)
				sendChar(c, drv);
		}
	}
	else
		cout << "[System]: Error" << endl;*/
	while (1)
    {
        int decision;
        cout << endl
             << "Was möchten Sie tun?\n[0] Empfangen\n[1] Senden\n[2] Exit" << endl;
        cin >> decision;

        if ((!cin.fail()) && (decision < 3 && decision > -1))
        {
            if(!decision)
                receiveChar(drv);
            if (decision)
            {
                if(decision == 2)
                    running = false;
                else
                {
                    charVector = dissectString(sentence);
					int size = charVector.size();
					sendEscape(drv);
					if (debug){cout << "charVector.size: " << size << endl;} // Debug
					sendChar((char)size,drv);
                }
            }
        }
	}
}

vector<char> dissectString(const string &sentence)
{
	vector<char> vector;
	if (sentence.empty())
	{
		cout << "Falsche Eingabe" << endl;
	}
	vector.assign(sentence.begin(), sentence.end());

	if (debug){cout << "size-dissectString: " << vector.size() << endl;} // debug
	return vector;
}

void sendChar(const char c, B15F &drv)
{

}

void receiveChar(B15F &drv)
{
	drv.setRegister(&DDRA, 0x00);
    // checking if escape was send
    bool active = false;
    while (!active)
    {
        active = checkEscape(drv);
    }

    vector<int> binary;
    for (int i = 0; i < 3; i++)
    {
        drv.delay_ms(500);
        binary.push_back((int)drv.getRegister(&PINA));
    }

    string s = "";
    for (int i = 0; i < 3; i++)
        s += std::bitset<3>(binary.at(i)).to_string();

    cout << "[System]: " << s << " empfangen" << endl;
    cout << "[System]: " << binToChar(s) << " (ASCII-Umwandlung)" << endl;
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
#include <iostream>
#include <b15f/b15f.h>
#include <vector>

using namespace std;

void sendEscape(B15F &);
bool checkEscape(B15F &);
void revieceChar(B15F &);
void sendChar(char, B15F &);
char binToChar(const string &);

int main()
{
    B15F &drv = B15F::getInstance();

    cout << endl
         << "-------[V-1.5]-------" << endl;
    bool running = true;
    while (running)
    {
        int decision;
        cout << endl
             << "Was möchten Sie tun?\n[0] Empfangen\n[1] Senden\n[2] Exit" << endl;
        cin >> decision;

        if ((!cin.fail()) && (decision < 3 && decision > -1))
        {
            if(!decision)
                revieceChar(drv);
            if (decision)
            {
                if(decision == 2)
                    running = false;
                else
                {
                    cout << "Bitte geben Sie einen Buchstaben zum Senden ein:" << endl;
                    char c;
                    cin >> c;
                    sendEscape(drv);
                    sendChar(c, drv);
                }
            }
        }
        // skipping wrong inputs
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    cout << endl << "[System]: Programm beendet" << endl << endl;
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

void revieceChar(B15F &drv)
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

void sendChar(char c, B15F &drv)
{
    drv.setRegister(&DDRA, 0x07);
    string binary = bitset<9>((int)c).to_string();
    cout << "Wert: " << c << endl
         << "Binary: " << binary << endl; // Debug

    for (long unsigned int i = 0; i < binary.length(); i += 3)
    {
        cout << "substring: " << binary.substr(i, 3) << endl; // Debug
        int bin = stoi(binary.substr(i, 3));
        drv.setRegister(&PORTA, bin);
        drv.delay_ms(500);
    }
}

char binToChar(const std::string &in)
{
    char temp = 0;
    for (int i = 0; i < 9; i++)
    {
        if ('1' == in.at(i))
        {
            temp += (int)pow(2, 8 - i);
        }
    }
    return temp;
}

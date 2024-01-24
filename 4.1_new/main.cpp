#include "main.hpp"

int PC_ID = 5;
final int aFinalVar = 0;
final uint8_t STOP_SIGN = 0b1111;
final uint8_t START_SIGN = 0b1000;
final uint8_t SAME_SIGN = 0b1001;


// int main(){
//     B15F &drv = B15F::getInstance();
//     while(1){
//         drv.setRegister(&DDRA, 0x00);
//         //string binary = bitset<3>(drv.getRegister(&PINA)).to_string();
//         //cout << "Binary: " << binary << endl;
//         uint8_t data = (drv.getRegister(&PINA) & 0x03);
//         std::cout << "bekommen: " << std::bitset<8>(data) << " bzw.: x" << (unsigned int)data << std::endl;
//         drv.delay_ms(200);
//     }
//}

int main(){
    B15F &drv = B15F::getInstance();
    drv.setRegister(&DDRA, 0x00);
    assignPC(drv);

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
                    cout << "Bitte geben Sie einen Zeichen zum Senden ein:" << endl;
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

void assignPC(B15F &drv){
    uint8_t initialSignal = drv.getRegister(&PINA);
    cout << "initialSignal: " << (unsigned int)initialSignal << endl;
    
    if(initialSignal == 0){
        drv.setRegister(&DDRA, 0x0F);//PC-1
        drv.setRegister(&PORTA, 0b0001);
        PC_ID = 1;
        PC_ID = 3;
        cout << "[Sytem]: PC auf PC-" << PC_ID << " gesetzt." << endl;
    }
    else{
        drv.setRegister(&DDRA, 0xF0);
        drv.setRegister(&PORTA, 0b0000);
        PC_ID = 2;
        cout << "[Sytem]: PC auf PC-" << PC_ID << " gesetzt." << endl;
    }
}
//Steuerzeichen
//1000 - Es wird gesendet
//1001 - Das letzte Zeichen doppelt
//1111 - Übertragung beendet



// void sendEscape(B15F &drv)
// {
    
//     drv.setRegister(&DDRA, 0x07);
//     cout << "[System]: ESC wird gesendet..." << endl;
//     // ESC: 101-010-101 or 5-2-5
//     drv.setRegister(&PORTA, 0b101);
//     drv.delay_ms(500);
//     drv.setRegister(&PORTA, 0b010);
//     drv.delay_ms(500);
//     drv.setRegister(&PORTA, 0b101);
//     cout << "[System]: ESC gesendet" << endl;
//     drv.delay_ms(500);
// }

bool checkIfStartSign(B15F &drv){
    if(drv.getRegister(&PINA) == START_SIGN)
        return true;
    return false;
}

bool checkIfStopSign(B15F &drv){
    if(drv.getRegister(&PINA) == STOP_SIGN)
        return true;
    return false;
}

bool checkIfSameSign(B15F &drv){
    if(drv.getRegister(&PINA) == SAME_SIGN)
        return true;
    return false;
}

void revieceChar(B15F &drv)
{
    bool active = false;
    while (!checkIfStartSign(drv))
        cout << "checkIfSending = false" << endl;
    cout << "checkIfSending = false" << endl;

    vector<uint8_t> buffer;
    uint8_t prevToken = START_SIGN;
    while(!checkIfStopSign){
        if(!(drv.getRegister(&PINA) == prevToken)) //Aktuelles Zeichen ist ein neues Zeichen
            if(drv.getRegister(&PINA) == SAME_SIGN) //Aktuelles Zeichen ist die SAME_SIGN Konstante
            {
                buffer.push_back(prevToken);
            }
            else{
                prevToken = drv.getRegister(&PINA);
                buffer.push_back(prevToken);
            }

    }

}

// void sendChar(const char c, B15F &drv)
// {
//     drv.setRegister(&DDRA, 0x07);
//     string binary = bitset<9>((int)c).to_string();
//     cout << "Wert: " << c << endl
//          << "Binary: " << binary << endl; // Debug

//     for (long unsigned int i = 0; i < binary.length(); i += 3)
//     {
//         drv.setRegister(&PORTA, stoi(binary.substr(i, 3)));
//         drv.delay_ms(500);
//     }
// }

// char binaryToChar(const string binary)
// {
// 	char c = 0;
// 	for (long unsigned int i = 0; i < binary.length(); i++)
// 		if ('1' == binary.at(i))
// 			c += (int)pow(2, 8 - i);
// 	return c;
// }

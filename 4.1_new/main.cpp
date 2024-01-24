#include "main.hpp"

int PC_ID = 5;
int aFinalVar = 0;
uint8_t STOP_SIGN = 0b1111;
uint8_t START_SIGN = 0b1000;
uint8_t SAME_SIGN = 0b1001;
int SENDING_CYCLE = 300;
int RECEIVING_CYCLE = 100;

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
//void assignPC(B15F&);

int main(){
    B15F &drv = B15F::getInstance();
    drv.setRegister(&DDRA, 0x00);
    
    int a = drv.getRegister(&PINA);

    //assignPC(drv);
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
                {
                    cout << "test1" << endl;
                    cout << "test4" << endl;
                    revieceChar(drv);
                    cout << "test3" << endl;
                }
            if (decision)
            {
                if(decision == 2)
                    running = false;
                else
                {
                    cout << "Bitte geben Sie einen Zeichen zum Senden ein:" << endl;
                    char c;
                    cin >> c;
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

void assignPC(B15F drv){ //bereitet Probleme
    uint8_t initialSignal = drv.getRegister(&PINA);
    
    if(initialSignal == 0){
        drv.setRegister(&DDRA, 0x0F);
        drv.setRegister(&PORTA, 0b0001);
        PC_ID = 1;
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

bool checkIfStartSign(B15F &drv){
    cout << (unsigned int)drv.getRegister(&PINA) << endl;
    if((int)drv.getRegister(&PINA) == 4)
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
    cout << "test1" << endl;
    while (!checkIfStartSign(drv))
    {
        cout << "checkIfSending = false" << endl;
        drv.delay_ms(RECEIVING_CYCLE);
    }
    cout << "checkIfSending = false" << endl;

    vector<uint8_t> buffer;
    uint8_t prevToken = START_SIGN;
    while(!checkIfStopSign(drv)){
        if(!(drv.getRegister(&PINA) == prevToken)) //Aktuelles Zeichen ist ein neues Zeichen
            if(drv.getRegister(&PINA) == SAME_SIGN) //Aktuelles Zeichen ist die SAME_SIGN Konstante
            {
                prevToken = SAME_SIGN; //prevToken wird SAME_SIGN, dieses wird aber nicht angehaengt
            }
            else{
                prevToken = drv.getRegister(&PINA); //prevToken wird das neue Zeichen
                buffer.push_back(prevToken); //neues Zeichen wird an den Vector angehaengt
            }
        drv.delay_ms(RECEIVING_CYCLE);
    }
    string s;
    for(int i : buffer) 
        s += buffer.at(i);
    cout << "buffer: " << s << endl;
}
void sendStartSign(B15F &drv){
    cout << "Startzeichen gesendet: " << (unsigned int)START_SIGN << endl;
    drv.setRegister(&PORTA, START_SIGN);
    drv.delay_ms(SENDING_CYCLE);
}

void sendStopSign(B15F &drv){
    drv.setRegister(&PORTA, STOP_SIGN);
    drv.delay_ms(SENDING_CYCLE);
}

void sendChar(const char c, B15F &drv)
{
    string binary = bitset<9>((int)c).to_string();
    cout << "Wert: " << c << endl
         << "Binary: " << binary << endl; // Debug

    sendStartSign(drv);
    for (long unsigned int i = 0; i < binary.length(); i += 3)
    {
        drv.setRegister(&PORTA, stoi(binary.substr(i, 3)));
        drv.delay_ms(SENDING_CYCLE);
    }
    sendStopSign(drv);
} 

// char binaryToChar(const string binary)
// {
// 	char c = 0;
// 	for (long unsigned int i = 0; i < binary.length(); i++)
// 		if ('1' == binary.at(i))
// 			c += (int)pow(2, 8 - i);
// 	return c;
// }

#include "main.hpp"

int PC_ID;
uint8_t const START_SIGN =  0b00001000;
uint8_t const STOP_SIGN =   0b00001001;
uint8_t const ACK_SIGN =    0b00001010;
uint8_t const DEN_SIGN =    0b00001011;
int const SENDING_CYCLE = 300;
int const RECEIVING_CYCLE = 100;
uint8_t tact = 0;
uint8_t prevTact = 0;


int main(){
    B15F &drv = B15F::getInstance();
	//assignPC(drv);	
	 
    bool running = true;
    sending(drv, 'a');
    while (running)
    {
        int decision;
        cout << endl
             << "Was möchten Sie tun?\n[0] PC-0\n[1] PC-1\n[2] Exit" << endl;
        cin >> decision;

        if ((!cin.fail()) && (decision < 3 && decision > -1))
        {
            if(!decision)
                {
                	//PC-0
                    drv.setRegister(&DDRA, 0x0F);
                    PC_ID = 0;
                    cout << "[Sytem]: PC auf PC-" << PC_ID << " gesetzt." << endl;
                    drv.setRegister(&PORTA, sendConverting(4));
                    while(1){
                        cout << (long int) reveiceConverting(drv.getRegister(&PINA)) << endl;
                        cout << bitset<8>(drv.getRegister(&PINA)) << endl;
                        drv.delay_ms(500);
                    }
                }
            if (decision)
            {
                if(decision == 2)
                    running = false;
                else
                {
                    //PC-1
                    drv.setRegister(&DDRA, 0xF0);
                    PC_ID = 1;
                    cout << "[Sytem]: PC auf PC-" << PC_ID << " gesetzt." << endl;
                    drv.setRegister(&PORTA, sendConverting(6));
                    while(1){
                        cout << (long int) reveiceConverting(drv.getRegister(&PINA)) << endl;
                        cout << bitset<8>(drv.getRegister(&PINA)) << endl;
                        drv.delay_ms(500);
                    }
                }
            }
        }
        // skipping wrong inputs
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    cout << endl << "[System]: Programm beendet" << endl << endl;
}

void assignPC(B15F &drv){ //bereitet Probleme
    uint8_t initialSignal = drv.getRegister(&PINA);
    
    if(initialSignal == 0){ 
        drv.setRegister(&DDRA, 0x0F);
        drv.setRegister(&PORTA, 0b0001);
        PC_ID = 0;
        cout << "[Sytem]: PC auf PC-" << PC_ID << " gesetzt." << endl;
    }
    else{
        drv.setRegister(&DDRA, 0xF0);
        drv.setRegister(&PORTA, 0b0000);
        PC_ID = 1;
        cout << "[System]: PC auf PC-" << PC_ID << " gesetzt." << endl;
    }
}

/**
* Diese Methode nutzt die gesetzte Nummer des PCs um anhand dieser
* eine Zahl so umzuwandeln, dass diese richtig über das Board gesendet
* wird. Die PC_ID kennzeichnet hier, wie das Registers des Boards an diesem
* PC gesetzt wurde. Da bei einem 0xF0-Register eine Binärzahl 1011 nicht gesendet werden
* würde, da die ersten 4 Bits nicht zum senden verwendet werden können, muss in diesem Fall
* die Zahl um 4 Bits nach links geshifted werden. Das Ergebnis ist dann 10110000, welches 
* dann vom Board richtig als 1011 gesendet wird.
*
* @param  drv  Eine Referenz des Boards
* @param  num  Die jeweilige Zahl, die konvertiert werden soll
* @return      Die Zahl konvertiert für die jeweilige PC-Nummer
*/
uint8_t sendConverting(uint8_t num){
    if(PC_ID == 1)
        return (num << 4);
    return num;
}

/**
* Diese Methode nutzt die gesetzte Nummer des PCs um anhand dieser
* eine Zahl so umzuwandeln, dass die über das Board übertragene Nummer
* richtig benutzt wird. Die PC_ID kennzeichnet hier, wie das Registers des Boards an diesem
* PC gesetzt wurde. Da bei einem 0xF0-Register eine Binärzahl 1011 als 10110000 ankommen würde,
* muss diese erst durch einen Shift von 4 Bits nach rechts in Ihre ursprüngliche Zahl konvertiert werden.
*
* @param  drv  Eine Referenz des Boards
* @param  num  Die jeweilige Zahl, die konvertiert werden soll
* @return      Die Zahl konvertiert für die jeweilige PC-Nummer
*/
uint8_t reveiceConverting(uint8_t num){
    if(PC_ID == 0) //0x0F
        return (num >> 4);
    return (num & 0x0F); //0xF0
}

void sending(B15F &drv, char c){
    uint8_t num = c;
    cout << "num: " << (long int) num << endl << "Bitset: " << bitset<8>(num) << endl;
    
    if(tact == prevTact)
        tact = !prevTact;

    vector<uint8_t> binarysAsVector = dissectBinary(num);
    for(uint8_t i : binarysAsVector)
        cout << "[" << (long int) i << "] ";
    cout << endl;

    prevTact = tact;
}

uint8_t addTactToPackage(uint8_t num){
    if(tact == 1)
        return num | 0b00000100;
    return num & 11111011;
}

vector<uint8_t> dissectBinary(uint8_t num)
{
    vector<uint8_t> binarysAsVector;
    for(long unsigned int i = 0; i < 4; i++)
        binarysAsVector.push_back(3 & (num >> i*2));
    return binarysAsVector;
}
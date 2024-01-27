#include "main.hpp"

int PC_ID;
uint8_t const START_SIGN = 0b00001000;
uint8_t const STOP_SIGN = 0b00001111;
uint8_t const SAME_SIGN = 0b00001010;
uint8_t const ACK_SIGN = 0b00001011;
uint8_t const DEN_SIGN = 0b00001001;
// uint8_t const X_SIGN =    0b00001100;
// uint8_t const Y_SIGN =    0b00001101;
// uint8_t const Z_SIGN =    0b00001110;
int const SENDING_CYCLE_TIME = 1000;
int const RECEIVING_CYCLE_TIME = 500;

int main(){
    B15F &drv = B15F::getInstance();
    bool running = true;
    
    string s = "ABC";
    vector<vector<uint8_t>> sentenceVector = createSentenceVector(s);
    print2dVector(sentenceVector);




    while (running)
    {
        int decision;
        cout << endl
             << "Was moechten Sie tun?\n[0] PC-0\n[1] PC-1\n[2] Exit" << endl;
        cin >> decision;
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if ((!cin.fail()) && (decision < 3 && decision > -1))
        {
            if (!decision)
            {
                // PC-0
                PC_ID = 0;
                drv.setRegister(&DDRA, 0x0F);
                cout << "PC-ID: " << PC_ID << endl;
                leitungstest(drv);
                sending(drv, sentenceVector);
            }
            if (decision)
            {
                if (decision == 2)
                    running = false;
                else
                {
                    // PC-1
                    PC_ID = 1;
                    drv.setRegister(&DDRA, 0xF0);
                    cout << "PC-ID: " << PC_ID << endl;
                    leitungstest(drv);
                    print2dVector(receiving(drv));
                    //while(1){
                    //     cout << "Empfangen: "<< bitset<4>(reveiceConverting(drv.getRegister(&PINA))) << endl;
                    //     drv.delay_ms(RECEIVING_CYCLE_TIME);
                    // }
                }
            }
        }
        // skipping wrong inputs
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    cout << endl
         << "[System]: Programm beendet" << endl
         << endl;
}

//Funktioniert nur wenn Leitung am Anfang auf 0
void assignPC(B15F &drv){
    uint8_t initialSignal = drv.getRegister(&PINA);
    cout << "initialSignal: " << bitset<8>(initialSignal) << endl;
    
    drv.setRegister(&DDRA, 0xFF);
    cout << "[System]: Leitung = " << bitset<8>(drv.getRegister(&PINA)) << endl;
    if (initialSignal == 0){
        PC_ID = 0;
        drv.setRegister(&DDRA, 0x0F);
        drv.setRegister(&PORTA, 1);
        cout << "[System]: Leitung auf 1 = " << bitset<8>(drv.getRegister(&PINA)) << endl;
    }
    else{
        drv.setRegister(&DDRA, 0xF0);
        drv.setRegister(&PORTA, 0);
        PC_ID = 1;
    }
    cout << "[System]: Leitung nach Register= " << bitset<8>(drv.getRegister(&PINA)) << endl;
    cout << "[System]: PC auf PC-" << PC_ID << " gesetzt." << endl;
}

/**
 * Diese Methode nutzt die gesetzte Nummer des PCs um anhand dieser
 * eine Zahl so umzuwandeln, dass diese richtig ueber das Board gesendet
 * wird. Die PC_ID kennzeichnet hier, wie das Registers des Boards an diesem
 * PC gesetzt wurde. Da bei einem 0xF0-Register eine Binaerzahl 1011 nicht
 * gesendet werden w체rde, da die ersten 4 Bits nicht zum senden verwendet werden
 * k철nnen, muss in diesem Fall die Zahl um 4 Bits nach links geshifted werden.
 * Das Ergebnis ist dann 10110000, welches dann vom Board richtig als 1011
 * gesendet wird.
 *
 * @param  drv  Eine Referenz des Boards
 * @param  num  Die jeweilige Zahl, die konvertiert werden soll
 * @return      Die Zahl konvertiert f체r die jeweilige PC-Nummer
 */
uint8_t sendConverting(uint8_t num){
    if (PC_ID == 1)
        return (num << 4);
    return num;
}

/**
 * Diese Methode nutzt die gesetzte Nummer des PCs um anhand dieser
 * eine Zahl so umzuwandeln, dass die 체ber das Board 체bertragene Nummer
 * richtig benutzt wird. Die PC_ID kennzeichnet hier, wie das Registers des
 * Boards an diesem PC gesetzt wurde. Da bei einem 0xF0-Register eine Bin채rzahl
 * 1011 als 10110000 ankommen w체rde, muss diese erst durch einen Shift von 4
 * Bits nach rechts in Ihre urspr체ngliche Zahl konvertiert werden.
 *
 * @param  drv  Eine Referenz des Boards
 * @param  num  Die jeweilige Zahl, die konvertiert werden soll
 * @return      Die Zahl konvertiert f체r die jeweilige PC-Nummer
 */
uint8_t reveiceConverting(uint8_t num){
    if (PC_ID == 0) // 0x0F
        return (num >> 4);
    return (num & 0x0F); // 0xF0
}

void sending(B15F &drv, vector<vector<uint8_t>> sentenceVector){

    for (unsigned int i = 0; i < sentenceVector.size(); i++){ //Satz beginnt
        drv.setRegister(&PORTA, sendConverting(START_SIGN)); // Startzeichen senden
        cout << "[sending]: START_SIGN gesendet" << endl;
        drv.delay_ms(SENDING_CYCLE_TIME);
        
        for (unsigned int j = 0; j < sentenceVector.at(i).size(); j++) { // While (Zeichen sind nicht leer)
            drv.setRegister(&PORTA,sendConverting(sentenceVector.at(i).at(j))); // aktuellesZeichen wird gesendet // Zeichen an i senden
            cout << "[sending]: Zeichen gesendet" << endl;
            if ((  (j < 2) // Das naechste Zeichen ist im aktuellen Vektor
                && (sentenceVector.at(i).at(j) == sentenceVector.at(i).at(j + 1))) // AktuellesZeichen == naechstes Zeichen
                ||  (((j >= 2)                            // Das naechste Zeichen ist im naechsten Vektor
                    && (i + 1 < sentenceVector.size())) // Der naechste Vektor existiert
                    && (sentenceVector.at(i).at(j) == sentenceVector.at(i + 1).at(0)))){ // Aktuelles Zeichen == naechstes Zeichen
                    drv.delay_ms(SENDING_CYCLE_TIME);
                    drv.setRegister(&PORTA, sendConverting(SAME_SIGN));
                    cout << "[sending]: SAME_SIGN gesendet" << endl;
                }

            drv.delay_ms(SENDING_CYCLE_TIME);
        }
    }
    drv.setRegister(&PORTA,sendConverting(STOP_SIGN)); // Stopzeichen senden (Uebertragung beendet)
    cout << "[sending]: STOP_SIGN gesendet" << endl;
}

vector<vector<uint8_t>> receiving(B15F &drv){
    vector<vector<uint8_t>> sentenceVector;
    uint8_t lastBitset;
    uint8_t currentBitset;

    while(reveiceConverting(drv.getRegister(&PINA)) != STOP_SIGN)
    {
        cout << "[receiving]: Kein STOP_SIGN empfangen" << endl;
        currentBitset = reveiceConverting(drv.getRegister(&PINA));
        if(currentBitset == START_SIGN)
        {
            cout << "[receiving]: START_SIGN empfangen" << endl;
            lastBitset = currentBitset;
            vector<uint8_t> binaryVector;
            while(binaryVector.size() < 3)
            {
                currentBitset = reveiceConverting(drv.getRegister(&PINA));
                cout << "[receiving]: currentBitset = " << bitset<4>(currentBitset) << "(" << (long int) currentBitset << ")" << endl;
                if(currentBitset != START_SIGN){
                    if(currentBitset == lastBitset){ //Leitung hat sich geaendert
                        cout << "[receiving]: currentBitset und lastbitset sind gleich" << endl;
                    }
                    else if(currentBitset == SAME_SIGN){ //Leitung ist SAME_SIGN
                        cout << "[receiving]: currentBitset == SAME_SIGN" << endl;
                        lastBitset = SAME_SIGN;
                    }
                    else{ //Leitung ist nicht SAME_SIGN
                        binaryVector.push_back(currentBitset);
                        lastBitset = currentBitset;
                        cout << "[receiving]: currentBitset " << bitset<4>(currentBitset) << " wurde angefuegt" << endl;
                    }
                }
                cout << "[receiving]: Ende" << endl; 
                cout << "[receiving]: currentBitset = " << bitset<8>(currentBitset) << "(" << (long int) currentBitset << ")" << endl;
                cout << "[receiving]: lastBitset = " << bitset<8>(lastBitset) << "(" << (long int) lastBitset << ")" << endl;
                drv.delay_ms(RECEIVING_CYCLE_TIME);
            }
            sentenceVector.push_back(binaryVector);
        }
        cout << "[receiving]: currentBitset != START_SIGN" << endl;
        drv.delay_ms(RECEIVING_CYCLE_TIME);
    }
    cout << "[receiving]: STOP_SIGN empfangen" << endl;
    return sentenceVector;
}

vector<uint8_t> createBinaryVector(uint8_t num){
    vector<uint8_t> binaryVector;
    for (long unsigned int i = 0; i < 3; i++)
        binaryVector.push_back(7 & (num >> (i * 3)));
    return binaryVector;
}

vector<vector<uint8_t>> createSentenceVector(const string s){
    vector<vector<uint8_t>> sentenceVector;
    for (unsigned int i = 0; i < s.length(); i++)
        sentenceVector.push_back(createBinaryVector(s.at(i)));
    return sentenceVector;
}

void setParity(vector<vector<uint8_t>> &sentenceVector){
    for (unsigned int i = 0; i < sentenceVector.size(); i++){
        int count = 0;
        for (unsigned int j = 0; j < sentenceVector.at(i).size(); j++)
            count += countOnes(sentenceVector.at(i).at(j));
        if (!isEven(count))
            sentenceVector.at(i).at(0) | 0b100;
    }
}

int countOnes(uint8_t num){
    int count = 0;
    while (num){
        count += (num & 1);
        num = num >> 1;
    }
    return count;
}

bool isEven(int num){
    return num % 2 == 0;
}

void leitungstest(B15F &drv){
    if(PC_ID == 0){
        drv.setRegister(&PORTA, sendConverting(0b0101));
        cout << "[System]: Output gesetzt ("<< bitset<8>(drv.getRegister(&PINA)) <<")\nLeitung testen? (Enter)";
        cin.ignore();

        if(reveiceConverting(drv.getRegister(&PINA)) == 0b1010)
            cout << "[System]: PC-0 - Leitung korrekt" << endl;
        else
            cout << "[FEHLER]: PC-0 - Fehler in der Leitung\n[FEHLER]: Soll-Wert: 1010\n[FEHLER]: Ist-Wert = "
            << bitset<4>(reveiceConverting(drv.getRegister(&PINA))) << endl;
    }
    else{
        drv.setRegister(&PORTA, sendConverting(0b1010));
        cout << "[System]: Output gesetzt ("<< bitset<8>(drv.getRegister(&PINA)) <<")\nLeitung testen? (Enter)";
        cin.ignore();
        
        if(reveiceConverting(drv.getRegister(&PINA)) == 0b0101)
            cout << "[System]: PC-1 - Leitung korrekt (Enter)" << endl;
        else
            cout << "[FEHLER]: PC-0 - Fehler in der Leitung\n[FEHLER]: Soll-Wert: 0101\n[FEHLER]: Ist-Wert = "
                << bitset<4>(reveiceConverting(drv.getRegister(&PINA))) << " (Enter)"<<endl;
    }
    drv.delay_ms(1000);
    cout << "[System]: Leitung: " << bitset<8>(drv.getRegister(&PINA)) << endl;
    drv.delay_ms(500);
    cout << "[System]: Leitung zurueckgesetzen (Enter)" << endl;

    cin.ignore();
    drv.setRegister(&PORTA, sendConverting(0));


}

void clearLeitung(B15F &drv){
        int decision;
        cout << endl
             << "Soll die Leitung auf 0 gesetzt werden?\n[0] Nein\n[1] Ja" << endl;
        cin >> decision;

        if (decision)
        {    
            drv.setRegister(&DDRA, 0xFF);
            drv.setRegister(&PORTA, 0);
            cout << "Leitung wurde auf 0 gesetzt" << endl;
        }
        
        // skipping wrong inputs
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void print2dVector(vector<vector<uint8_t>> sentenceVector){
    cout << "Vektorgr붳en:  V1 = " << sentenceVector.size() << " V2 = " << sentenceVector.at(0).size() << endl;
    for (unsigned int i = 0; i < sentenceVector.size(); i++){
        cout << "     |" << endl;
        for (unsigned int j = 0; j < sentenceVector.at(i).size(); j++)
            cout << "[" << bitset<3>(sentenceVector.at(i).at(j)) << "] ";
        cout << endl;
    }
}
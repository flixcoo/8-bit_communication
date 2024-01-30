#include "main.hpp"

// Sende-PC
// cat <datei> | ./main.elf
// Empfangs-PC
// ./main.elf

int PC_ID;
uint8_t const START_SIGN =  0b00001000;
uint8_t const STOP_SIGN =   0b00001111;
uint8_t const SAME_SIGN =   0b00001010;
uint8_t const ACK_SIGN =    0b00001011;
uint8_t const DEN_SIGN =    0b00001001;
uint8_t const REQ_SIGN =    0b00001100;
// uint8_t const X_SIGN =    0b00001101;
// uint8_t const Y_SIGN =    0b00001110;
int const SENDING_CYCLE_TIME_MS = 300;
int const RECEIVING_CYCLE_TIME_MS = 100;


int main(){

    B15F &drv = B15F::getInstance();
    bool running = true;
    ofstream outputFile;
    outputFile.open("received.txt");
    vector<vector<uint8_t>> sentenceVector;
    
    auto current = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(current);
    
    if(!isatty(fileno(stdin))){
        PC_ID = 0;
        cout << "[System]: PC-ID = " << PC_ID << endl;
        drv.setRegister(&DDRA, 0x0F);
        outputFile << ctime(&currentTime) << endl;
        //leitungstest(drv);
        string inputString;
        string line;
        while(getline(cin, line)){
            inputString += line;
        }
        cout << "[System]: Input = " << inputString << endl;
        sentenceVector = createSentenceVector(inputString);
        if(sendRequest(drv))
            sending(drv, sentenceVector);
    }
    else{
        PC_ID = 1;
        cout << "[System]: PC-ID = " << PC_ID << endl;
        drv.setRegister(&DDRA, 0xF0);
        //leitungstest(drv);
        cout << "[Comm]: Waiting for Request..." << endl;
        while(running){
            if(reveiceConverting(drv.getRegister(&PINA)) == REQ_SIGN){
                drv.setRegister(&PORTA, sendConverting(ACK_SIGN));
                sentenceVector = receiving(drv);
                running = false;
            } 
        }
        cout << "Empfangen: ";
        for(unsigned int i = 0; i < sentenceVector.size(); i++){
            outputFile << mergeBitsets(sentenceVector.at(i));
            cout << mergeBitsets(sentenceVector.at(i));
        }
        cout << endl;
    }
    outputFile.close();
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
 * gesendet werden wuerde, da die ersten 4 Bits nicht zum senden verwendet werden
 * können, muss in diesem Fall die Zahl um 4 Bits nach links geshifted werden.
 * Das Ergebnis ist dann 10110000, welches dann vom Board richtig als 1011
 * gesendet wird.
 *
 * @param  drv  Eine Referenz des Boards
 * @param  num  Die jeweilige Zahl, die konvertiert werden soll
 * @return      Die Zahl konvertiert fuer die jeweilige PC-Nummer
 */
uint8_t sendConverting(uint8_t num){
    if (PC_ID == 1)
        return (num << 4);
    return num;
}

/**
 * Diese Methode nutzt die gesetzte Nummer des PCs um anhand dieser
 * eine Zahl so umzuwandeln, dass die ueber das Board uebertragene Nummer
 * richtig benutzt wird. Die PC_ID kennzeichnet hier, wie das Registers des
 * Boards an diesem PC gesetzt wurde. Da bei einem 0xF0-Register eine Binaerzahl
 * 1011 als 10110000 ankommen wuerde, muss diese erst durch einen Shift von 4
 * Bits nach rechts in Ihre urspruengliche Zahl konvertiert werden.
 *
 * @param  drv  Eine Referenz des Boards
 * @param  num  Die jeweilige Zahl, die konvertiert werden soll
 * @return      Die Zahl konvertiert fuer die jeweilige PC-Nummer
 */
uint8_t reveiceConverting(uint8_t num){
    if (PC_ID == 0) // 0x0F
        return (num >> 4);
    return (num & 0x0F); // 0xF0
}

void sending(B15F &drv, vector<vector<uint8_t>> sentenceVector){
    setParity(sentenceVector);
    
    for (unsigned int i = 0; i < sentenceVector.size(); i++){ //Satz beginnt
        drv.setRegister(&PORTA, sendConverting(START_SIGN)); // Startzeichen senden
        cout << "[sending]: START_SIGN gesendet" << endl;
        drv.delay_ms(SENDING_CYCLE_TIME_MS);
        for (unsigned int j = 0; j < sentenceVector.at(i).size(); j++) { // While (Zeichen sind nicht leer)
            drv.setRegister(&PORTA,sendConverting(sentenceVector.at(i).at(j))); // aktuellesZeichen wird gesendet // Zeichen an i senden
            cout << "[sending]: Zeichen " << bitset<3>(sentenceVector.at(i).at(j)) << " gesendet" << endl;
            if ((  (j < 2) // Das naechste Zeichen ist im aktuellen Vektor
                && (sentenceVector.at(i).at(j) == sentenceVector.at(i).at(j + 1))) // AktuellesZeichen == naechstes Zeichen
                ||  (((j >= 2)                            // Das naechste Zeichen ist im naechsten Vektor
                    && (i + 1 < sentenceVector.size())) // Der naechste Vektor existiert
                    && (sentenceVector.at(i).at(j) == sentenceVector.at(i + 1).at(0)))){ // Aktuelles Zeichen == naechstes Zeichen
                    drv.delay_ms(SENDING_CYCLE_TIME_MS);
                    drv.setRegister(&PORTA, sendConverting(SAME_SIGN));
                    cout << "[sending]: SAME_SIGN gesendet" << endl;
                }
                drv.delay_ms(SENDING_CYCLE_TIME_MS);
        }
        if(reveiceConverting(drv.getRegister(&PINA)) == DEN_SIGN){
            cout << "==== Uebertragung fehlgeschlagen ====" << endl;
            i -= 1;
        }
        else if(reveiceConverting(drv.getRegister(&PINA)) == ACK_SIGN)
            cout << "==== Uebertragung erfolgreich ====" << endl;
    }
    drv.setRegister(&PORTA,sendConverting(STOP_SIGN)); // Stopzeichen senden (Uebertragung beendet)
    cout << "[sending]: STOP_SIGN gesendet" << endl;
}

vector<vector<uint8_t>> receiving(B15F &drv){
    vector<vector<uint8_t>> sentenceVector;
    uint8_t lastBitset;
    uint8_t currentBitset;
    auto start = std::chrono::system_clock::now();
    while(reveiceConverting(drv.getRegister(&PINA)) != STOP_SIGN)
    {
        currentBitset = reveiceConverting(drv.getRegister(&PINA));
        if(currentBitset == START_SIGN)
        {
            cout << "[receiving]: START_SIGN empfangen" << endl;
            lastBitset = currentBitset;
            vector<uint8_t> binaryVector;
            drv.setRegister(&PORTA, sendConverting(0b0000));
            while(binaryVector.size() < 3)
            {
                currentBitset = reveiceConverting(drv.getRegister(&PINA));
                //cout << "[receiving]: currentBitset = " << bitset<4>(currentBitset) << "(" << (long int) currentBitset << ")" << endl;
                if(currentBitset != START_SIGN){
                    if(currentBitset == lastBitset){ //Leitung hat sich geaendert
                       cout << "[receiving]: Kein neues Zeichen" << endl;
                    }else if(currentBitset == SAME_SIGN){ //Leitung ist SAME_SIGN
                        cout << "[receiving]: SAME_SIGN empfangen" << endl;
                        lastBitset = SAME_SIGN;
                    }else{ //Leitung ist nicht SAME_SIGN
                        binaryVector.push_back(currentBitset);
                        lastBitset = currentBitset;
                        cout << "[receiving]: Neues Zeichen " << bitset<3>(currentBitset) << " wurde angefuegt" << endl;
                    }
                }
                drv.delay_ms(RECEIVING_CYCLE_TIME_MS);
            }
            if(checkParity(binaryVector)){
                drv.setRegister(&PORTA, sendConverting(ACK_SIGN));
                cout << "[Comm]: ACKNOWDLEGDE gesendet" << endl;
                removeParity(binaryVector);
                sentenceVector.push_back(binaryVector);
            }
            else{
                cout << "[Comm]: DENY gesendet" << endl;
                drv.setRegister(&PORTA, sendConverting(DEN_SIGN));
            }
        }
        drv.delay_ms(RECEIVING_CYCLE_TIME_MS);
    }
    cout << "[receiving]: STOP_SIGN empfangen" << endl;
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "[receiving]: Elapsed time: " << elapsed_seconds.count() << "s" << std::endl;
    drv.setRegister(&PORTA, sendConverting(0));
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

vector<vector<uint8_t>> setParity(vector<vector<uint8_t>> &sentenceVector){
    for (unsigned int i = 0; i < sentenceVector.size(); i++){
        int count = 0;
        for (unsigned int j = 0; j < sentenceVector.at(i).size(); j++)
            count += countOnes(sentenceVector.at(i).at(j));
        if (!isEven(count))
            (sentenceVector.at(i).at(2) |= 0b100);
    }
    return sentenceVector;
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

bool checkParity(vector<uint8_t> binaryVector){
    uint8_t parity = (binaryVector.at(2) >> 2);
    removeParity(binaryVector); //Paritaet löschen
    int count = 0;
    for(unsigned int i = 0; i < binaryVector.size(); i++)
        count += countOnes(binaryVector.at(i));
    if(parity != isEven(count))
        return true;
    return false;
}

void removeParity(vector<uint8_t> &binaryVector){
    binaryVector.at(2) &= 0b011;
}

void leitungstest(B15F &drv){
    cin.clear();
    //cin.ignore(numeric_limits<streamsize>::max(), '\n');
    if(PC_ID == 0){
        drv.setRegister(&PORTA, sendConverting(0b0101));
        cout << "[System]: Output gesetzt ("<< bitset<8>(drv.getRegister(&PINA)) <<")\nLeitung testen? (Enter)";
        cin.ignore();

        if(reveiceConverting(drv.getRegister(&PINA)) == 0b1010)
            cout << "[System]: PC-0 - Leitung korrekt" << endl;
        else
            cout << "[FEHLER]: PC-1 - Fehler in der Leitung\n[FEHLER]: Soll-Wert: 1010\n[FEHLER]: Ist-Wert = "
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

void printSentenceVector(vector<vector<uint8_t>> sentenceVector){
    for (unsigned int i = 0; i < sentenceVector.size(); i++){
        cout << "i:0     1     2" << endl;
        for (unsigned int j = 0; j < sentenceVector.at(i).size(); j++)
            cout << "[" << bitset<3>(sentenceVector.at(i).at(j)) << "] ";
        cout << endl;
    }
}

void printBinaryVectorToChar(vector<uint8_t> vector){
    for(unsigned int i = 0; i < vector.size(); i++)
        cout << vector.at(i);
}

uint8_t mergeBitsets(vector<uint8_t> binaryVector){
    uint8_t mergedNum = 0b00000000;
    for (unsigned int i = 0; i < binaryVector.size(); i++)
        mergedNum |= (binaryVector.at(i) << (i * 3));
    return mergedNum;
}

bool sendRequest(B15F &drv){
    drv.setRegister(&PORTA,sendConverting(REQ_SIGN));
    for(int i = 0; i < 20; i++){
        //cout << "ACK: " << bitset<4>(reveiceConverting(drv.getRegister(&PINA))) << endl;
        if(reveiceConverting(drv.getRegister(&PINA)) == ACK_SIGN){
            cout << "[Comm]: >>Uebertragung akzeptiert<<" << endl;
            return true;
        }
        else{
            cout << "[Comm]: Waiting for Acknowledge..." << endl;
        }
        drv.delay_ms(500);
    }
    cout << "[Comm]: Error - Request Timout" << endl;
    return false;
}
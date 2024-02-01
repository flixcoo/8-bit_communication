/* =====[ Instructions ]=====
* Zum Starten des Programmes muss auf dem Empfangs-PC
* das Programm ueber ./main.elf gestartet werden. Dieser wartet dann
* auf eine Anfrage (Programm terminiert nicht). 
* Auf dem Sende-PC muss das Programm mit einer hereingepipten Datei
* gestartet werden, (Bsp.: cat 100b.bin | ./main.elf) .
* Dieser PC wartet dann insgesamt 40 * 500ms (20 Sekunden) auf eine
* Bestaetigung durch den Empfangenden-PC
* 
* WICHTIG: Um einen optimalen Ablauf zu gewaehrleisten, sollten am Anfang
*          beide Leitungen auf 0 gesetzt sein (Labornetzteil einmal an und wieder aus machen).
* 
* INFO: Standardmäßig wird am Ende die Zeichenkette in der Konsole ausgegeben.
*       Um diese in eine Datei zu bekommen, fuehre auf dem Empfangs-PC der Befehl
*       vollgendermaßen aussehen: ./main.elf > output.txt (Name frei waehlbar).
*       Alle Konsolenausgaben im Programm laufen über std::cerr, nur die Ausgabe
*       der Zeichenkette laeuft ueber std::cout.
* 
* =====[ Instructions ]===== */

#include "main.hpp"

int PC_ID;
//Konstanten fuer die Steuerzeichen
uint8_t const START_SIGN =  0b00001000;
uint8_t const STOP_SIGN =   0b00001111;
uint8_t const SAME_SIGN =   0b00001010;
uint8_t const ACK_SIGN =    0b00001011;
uint8_t const DEN_SIGN =    0b00001001;
uint8_t const REQ_SIGN =    0b00001100;
//Zeit-Konstanten fuer den Sende- bzw Empfangszyklus in Millisekunden
int const SENDING_CYCLE_TIME_MS = 90;
int const RECEIVING_CYCLE_TIME_MS = 30;


int main(){

    B15F &drv = B15F::getInstance();
    bool running = true;
    vector<vector<uint8_t>> sentenceVector;
    
    if(!isatty(fileno(stdin))){ //Wenn eine Datei gepipied wurde (= PC ist Sende-PC)
        PC_ID = 0;
        cerr << "[System]: PC-ID = " << PC_ID << endl;
        drv.setRegister(&DDRA, 0x0F); //Setze Register fuer Sende-/Empfangsverhalten
        string inputString, line;
        while(getline(cin, line)) //Iteriere durch die komplette Input-Datei
            inputString += line;
        cerr << "[System]: Input = " << inputString << endl;
        sentenceVector = createSentenceVector(inputString); //Erstelle Variable fuer Zeichenkette
        if(sendRequest(drv))                //Wenn Request bestaetigt,
            sending(drv, sentenceVector);   //Sende Zeichenkette
    }
    else{ //Wenn eine Datei gepipied wurde (= PC ist Empfangs-PC)
        PC_ID = 1;
        cerr << "[System]: PC-ID = " << PC_ID << endl;
        drv.setRegister(&DDRA, 0xF0); //Setze Register fuer Sende-/Empfangsverhalten
        cerr << "[Comm]: Waiting for Request..." << endl;
        while(running){
            if(reveiceConverting(drv.getRegister(&PINA)) == REQ_SIGN){ //Warte auf Request
            	cerr << "[Comm]: >>Uebertragung akzeptiert<<" << endl;
                drv.setRegister(&PORTA, sendConverting(ACK_SIGN)); //Bestaetige Request
                sentenceVector = receiving(drv); //Empfange Zeichenkette in Variable
                running = false;
            } 
        }
        for(unsigned int i = 0; i < sentenceVector.size(); i++){
            cout << mergeBitsets(sentenceVector.at(i));
        }
        cerr << endl;
    }
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
 * @param  drv  Ein Objekt als Referenz auf eine Instanz des B15-Boards.
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
 * @param  drv  Ein Objekt als Referenz auf eine Instanz des B15-Boards.
 * @param  num  Die jeweilige Zahl, die konvertiert werden soll
 * @return      Die Zahl konvertiert fuer die jeweilige PC-Nummer
 */
uint8_t reveiceConverting(uint8_t num){
    if (PC_ID == 0) // 0x0F
        return (num >> 4);
    return (num & 0x0F); // 0xF0
}

/**
 * Diese Methode dient dem Senden einer Zeichenkette ueber eine 8-Bit leitung.
 * Hierzu wird ein Satzvektor uebergeben. Diesem wird zuerst die Paritaet
 * gesetzt. Danach wird jedes Zeichen, welches aus drei Paketen besteht,
 * mit einem vorangegangene Startzeichen gesendet. Sollte das naechste zusendende
 * Zeichen das gleiche sein, wie das vorher gesendete Zeichen, so wird dazwischen
 * ein SAME_SIGN gesendet. Nachdem drei Pakete uebertragen wurden, wird
 * die Leitung auf ein ACK_SIGN oder DEN_SIGN ueberprueft. Bei einem Deny
 * wird im Satzvektor ein Zeichen zurueckgesprungen, so dass das fehlgeschlagene Zeichen
 * erneut gesendet wird. Sobald durch den Satzvektor vollstaendig iteriert wurde,
 * wird ein STOP_SIGN gesendet.
 *
 * @param  drv             Ein Objekt als Referenz auf eine Instanz des B15-Boards.
 * @param  sentenceVector  Ein Satzvektor, welcher gesendet werden soll.
 * @return                 Kein Rueckgabewert, nur Konsolenoutput.
 */
void sending(B15F &drv, vector<vector<uint8_t>> sentenceVector){
    setParity(sentenceVector);
    
    for (unsigned int i = 0; i < sentenceVector.size(); i++){ //Satz beginnt
        drv.setRegister(&PORTA, sendConverting(START_SIGN)); // Startzeichen senden
        cerr << "[sending]: START_SIGN gesendet" << endl;
        drv.delay_ms(SENDING_CYCLE_TIME_MS);
        for (unsigned int j = 0; j < sentenceVector.at(i).size(); j++) { // While (Zeichen sind nicht leer)
            drv.setRegister(&PORTA,sendConverting(sentenceVector.at(i).at(j))); // aktuellesZeichen wird gesendet // Zeichen an i senden
            cerr << "[sending]: Zeichen " << bitset<3>(sentenceVector.at(i).at(j)) << " gesendet" << endl;
            if ((  (j < 2) // Das naechste Zeichen ist im aktuellen Vektor
                && (sentenceVector.at(i).at(j) == sentenceVector.at(i).at(j + 1))) // AktuellesZeichen == naechstes Zeichen
                ||  (((j >= 2)                            // Das naechste Zeichen ist im naechsten Vektor
                    && (i + 1 < sentenceVector.size())) // Der naechste Vektor existiert
                    && (sentenceVector.at(i).at(j) == sentenceVector.at(i + 1).at(0)))){ // Aktuelles Zeichen == naechstes Zeichen
                    drv.delay_ms(SENDING_CYCLE_TIME_MS);
                    drv.setRegister(&PORTA, sendConverting(SAME_SIGN));
                    cerr << "[sending]: SAME_SIGN gesendet" << endl;
                }
                drv.delay_ms(SENDING_CYCLE_TIME_MS);
        }
        if(reveiceConverting(drv.getRegister(&PINA)) == DEN_SIGN){
            cerr << "====  fehlgeschlagen ====" << endl;
            i -= 1;
        }
        else if(reveiceConverting(drv.getRegister(&PINA)) == ACK_SIGN)
            cerr << "==== Uebertragung erfolgreich ====" << endl;
    }
    drv.setRegister(&PORTA,sendConverting(STOP_SIGN)); // Stopzeichen senden (Uebertragung beendet)
    cerr << "[sending]: STOP_SIGN gesendet" << endl;
}

/**
 * Diese Funktion dient dem Empfangen einer Zeichenkette ueber eine 8-Bit Leitung.
 * Hierzu wird werden am Anfang u.a. die Rueckgabevariable sentenceVector erstellt.
 * Solange kein STOP_SIGN empfangen wird, wird auf ein START_SIGN gewartet. Sobald dieses kommt,
 * wird immer das currentBitset und das lastBitset genutzt, um eine Aenderung an
 * der Leitung festzustellen. Auch wird nach dem Steuerzeichen SAME_SIGN untersucht,
 * um moegliche doppelte Zeichen festzustellen. Dies wird wiederholt, bis drei Pakete
 * empfangen wurden. Nach drei Paketen wird die Paritaet der empfangenen Pakete geprueft.
 * Sollte diese Richtig sein, wird an den Sender ein Acknowledge (ACK_SIGN) gesendet,
 * die Paritaet entfernt und das neue Zeichen in den Satzvektor integriert.
 * Sollte die Paritaet falsch sein, so wird ein Deny (DEN_SIGN) gesendet.
 * Das Zeichen wird nicht in den Satzvektor uebernommen.
 * 
 * @param drv  Ein Objekt als Referenz auf eine Instanz des B15-Boards.
 * @return     Ein Satzvektor, welche die Empfangenen Zeichen enthaelt.
*/
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
            cerr << "[receiving]: START_SIGN empfangen" << endl;
            lastBitset = currentBitset;
            vector<uint8_t> binaryVector;
            drv.setRegister(&PORTA, sendConverting(0b0000));
            while(binaryVector.size() < 3)
            {
                currentBitset = reveiceConverting(drv.getRegister(&PINA));
                //cerr << "[receiving]: currentBitset = " << bitset<4>(currentBitset) << "(" << (long int) currentBitset << ")" << endl;
                if(currentBitset != START_SIGN){
                    if(currentBitset == lastBitset){ //Leitung hat sich geaendert
                       cerr << "[receiving]: Kein neues Zeichen" << endl;
                    }else if(currentBitset == SAME_SIGN){ //Leitung ist SAME_SIGN
                        cerr << "[receiving]: SAME_SIGN empfangen" << endl;
                        lastBitset = SAME_SIGN;
                    }else{ //Leitung ist nicht SAME_SIGN
                        binaryVector.push_back(currentBitset);
                        lastBitset = currentBitset;
                        cerr << "[receiving]: Neues Zeichen " << bitset<3>(currentBitset) << " wurde empfangen" << endl;
                    }
                }
                drv.delay_ms(RECEIVING_CYCLE_TIME_MS);
            }
            if(checkParity(binaryVector)){
                drv.setRegister(&PORTA, sendConverting(ACK_SIGN));
                cerr << "[Comm]: ACKNOWDLEGDE gesendet" << endl;
                removeParity(binaryVector);
                sentenceVector.push_back(binaryVector);
            }
            else{
                cerr << "[Comm]: DENY gesendet" << endl;
                drv.setRegister(&PORTA, sendConverting(DEN_SIGN));
            }
        }
        drv.delay_ms(RECEIVING_CYCLE_TIME_MS);
    }
    cerr << "[receiving]: STOP_SIGN empfangen" << endl;
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cerr << "[receiving]: Elapsed time: " << elapsed_seconds.count() << "s" << std::endl;

    drv.setRegister(&PORTA, sendConverting(0));
    return sentenceVector;
}

/**
 * Erstellt aus einer Binärzahl einen Vektor in welchem die Binärzahl in 3-Bit-Blöcle zerteilt wird.
 *  An jedem Index (0,1,2) befindet sich eine uint8_t (8 Bit Binearzahl),
 * welche auf den ersten drei Bits einen Teil der eingegebenen Binärzahl
 * enthält. An Index 0 befinden sich die Bit 0 - 2, an Index 1 die Bits 3 - 5
 * und an Index 2 die Bits 6 + 7. Dies wird im späteren Verlauf genutzt um die
 * maxmimal 3 Bit großen Zahlen als Paket ueber die 4 Leitungen zu senden.
 * 
 * @param  num  Eine 8-Bit Binaerzahl, welche Zerteilt werden soll.
 * @return      Einen Vektor der Groeße 3 welcher die Teilbitsets enthaelt
 */
vector<uint8_t> createBinaryVector(uint8_t num){
    vector<uint8_t> binaryVector;
    for (long unsigned int i = 0; i < 3; i++)
        binaryVector.push_back(7 & (num >> (i * 3)));
    return binaryVector;
}

/**
 * Erstellt aus einem String einen Vektor welcher Binaervektor mit Teilbits enthaelt.
 * Jedes Zeichen aus dem String s wird wird durch createBinaryVector()
 * in maximal drei Bit große Bitsets zerteilt und danach angehaengt an den
 * sentenceVector, welcher somit ein Vektor ist, welcher Vektoren enthaelt,
 * welche Teilbitsets der Zeichen vom String s enthalten.
 * Diese Funktion dient spaeter zum einfacheren iterieren durch die
 * einzelnen Zeichen.
 * 
 * @param s  Eine Zeichenkette, welche zerlegt werden soll.
 * @return   Einen Vektor welcher einen Vektor mit Teilbitsets der zeichen von s enthaelt.
 */
vector<vector<uint8_t>> createSentenceVector(const string s){
    vector<vector<uint8_t>> sentenceVector;
    for (unsigned int i = 0; i < s.length(); i++)
        sentenceVector.push_back(createBinaryVector(s.at(i)));
    return sentenceVector;
}

/**
 * Setzt die korrekte Paritaet fuer jedes Zeichen im sentenceVector.
 * Dafuer wird durch den sentenceVector iteriert, und von jedem Teilbitvector
 * das Vorkommen des Bits '1' gezaehlt. Wenn diese Anzahl ungerade ist, wird
 * auf dem "9. Bit" eine Paritaet von 1 gesetzt, andernfalls 0.
 * Der sentenceVector wird als Referenz in die Methode gegeben,
 * weshalb kein Rueckggabewert notwendig ist.
 * 
 * @param sentenceVector Ein Satzvektor, dessen Paritaet gesetzt werden soll. 
 * @return               Keine Rueckgabe, da mit einer Referenz gearbeitet wird.
 */
void setParity(vector<vector<uint8_t>> &sentenceVector){
    for (unsigned int i = 0; i < sentenceVector.size(); i++){
        int count = 0;
        for (unsigned int j = 0; j < sentenceVector.at(i).size(); j++)
            count += countOnes(sentenceVector.at(i).at(j));
        if (!isEven(count))
            (sentenceVector.at(i).at(2) |= 0b100);
    }
}

/**
 * Zaehlt die Vorkommen des Bits '1' in einem einer 8 Bit Binaerzahl. 
 * Hierzu wird die Binaerzahl mit 00000001 verundet. Die uebrig gebliebene
 * Zahl wird auf die 'count'-Variable aufaddiert, d.h. im Falle einer '1'
 * wird der Count erhoeht, ansonnsten bleibt er gleich. Die Binaerzahl wird dann
 * um einen Bit nach rechts geshifted, so dass das naechste Bit an Position 0 ist.
 * Dies wird solange gemacht, wie die Binaerzahl Einsen enthaelt.
 * 
 * @param num    Eine 8-Bit Binaerzahl.
 * @return       Die Anzahl der Bits welche '1' sind
 */
int countOnes(uint8_t num){
    int count = 0;
    while (num){
        count += (num & 1);
        num = num >> 1;
    }
    return count;
}

/**
 * Zeigt, ob eine Zahl gerade ist.
 * Die eingegebene Zahl wird durch zwei geteilt.
 * Wenn der Rest null ist, geht die Gleichung im Return-Statement
 * auf und es wird true zurueckgegeben. Wenn es einen
 * Rest gibt, wird die Gleichung falsch und es wird
 * false zurueckgegeben.
 * 
 * @param num    Eine 8-Bit Binaerzahl.
 * @return       Wahrheitswert, ob die Zahl gerade ist.
 */
bool isEven(int num){
    return num % 2 == 0;
}

/**
 * Untersucht, ob die Paritaet einer Binaerzahl korrekt ist.
 * Hierzu wird die Paritaet seperat gespeichert, danach wird
 * sie vom geloesch und es werden die Anzahl der Bits mit '1'
 * gezaehlt. Wenn die Anzahl der '1' gerade bzw. ungerade ist und die Paritaet
 * auch 0 bzw. 1 ist, wird true zurueckgegeben, andernfalls false
 * 
 * @param binaryVector   Ein Vektor mit drei Paketen einer Binaerzahl und gesetzem Paritaetsbit.
 * @return               Wahrheitswert, ob die Paritaet stimmt.
 */
bool checkParity(vector<uint8_t> binaryVector){
    uint8_t parity = (binaryVector.at(2) >> 2);
    removeParity(binaryVector); 
    int count = 0;
    for(unsigned int i = 0; i < binaryVector.size(); i++)
        count += countOnes(binaryVector.at(i));
    if(parity != isEven(count))
        return true;
    return false;
}

/**
 * Untersucht, ob die Paritaet einer Binaerzahl korrekt ist.
 * Hierzu wird die Paritaet seperat gespeichert, danach wird
 * sie vom geloesch und es werden die Anzahl der Bits mit '1'
 * gezaehlt. Wenn die Anzahl der '1' gerade bzw. ungerade ist und die Paritaet
 * auch 0 bzw. 1 ist, wird true zurueckgegeben, andernfalls false.
 * 
 * @param binaryVector   Ein Vektor mit drei Paketen einer Binaerzahl und gesetzem Paritaetsbit.
 * @return               Wahrheitswert, ob die Paritaet stimmt.
*/
void removeParity(vector<uint8_t> &binaryVector){
    binaryVector.at(2) &= 0b011;
}

/**
 * Debug-Methode zum Testen der Leitungen.
 * Von Beiden Boards aus wird die Leitung auf einen festen Wert gesetzt und dieser
 * Wert wird vom jeweils anderen Board abgefragt. Wenn das richtige Zeichen empfangen
 * wurde, wird dies in der Konsole bestaetigt. Wenn das falsche Zeichen empfangen wurde,
 * wird neben einem Hinweis auch Ist- und Soll-Wert der Leitung ausgegeben.
 * Nach dem Test wird die Leitung von beiden Boards wieder auf 0 gesetzt.
 * 
 * @param drv  Ein Objekt als Referenz auf eine Instanz des B15-Boards.
 * @return     Keine Rueckggabe, nur Konsolenausgaben.
 */
void leitungstest(B15F &drv){
    cin.clear();
    if(PC_ID == 0){
        drv.setRegister(&PORTA, sendConverting(0b0101));
        cerr << "[System]: Output gesetzt ("<< bitset<8>(drv.getRegister(&PINA)) <<")\nLeitung testen? (Enter)";
        cin.ignore();

        if(reveiceConverting(drv.getRegister(&PINA)) == 0b1010)
            cerr << "[System]: PC-0 - Leitung korrekt" << endl;
        else
            cerr << "[FEHLER]: PC-1 - Fehler in der Leitung\n[FEHLER]: Soll-Wert: 1010\n[FEHLER]: Ist-Wert = "
            << bitset<4>(reveiceConverting(drv.getRegister(&PINA))) << endl;
    }
    else{
        drv.setRegister(&PORTA, sendConverting(0b1010));
        cerr << "[System]: Output gesetzt ("<< bitset<8>(drv.getRegister(&PINA)) <<")\nLeitung testen? (Enter)";
        cin.ignore();
        
        if(reveiceConverting(drv.getRegister(&PINA)) == 0b0101)
            cerr << "[System]: PC-1 - Leitung korrekt (Enter)" << endl;
        else
            cerr << "[FEHLER]: PC-0 - Fehler in der Leitung\n[FEHLER]: Soll-Wert: 0101\n[FEHLER]: Ist-Wert = "
                << bitset<4>(reveiceConverting(drv.getRegister(&PINA))) << " (Enter)"<<endl;
    }
    drv.delay_ms(1000);
    cerr << "[System]: Leitung: " << bitset<8>(drv.getRegister(&PINA)) << endl;
    drv.delay_ms(500);
    cerr << "[System]: Leitung zurueckgesetzen (Enter)" << endl;

    cin.ignore();
    drv.setRegister(&PORTA, sendConverting(0));
}

/**
 * Gibt den sentenceVector in der Konsole aus.
 * Diese Funktion ist zu Debug-Zwecken und gibt den sentenceVector
 * in einer uebersichtlichen Formatierung aus.
 * 
 * @param sentenceVector Ein Satzvektor, welcher in der Konsole ausgegeben werden soll.
 * @return               Keine Rueckgabe, nur Konsolenoutput.
 */
void printSentenceVector(vector<vector<uint8_t>> sentenceVector){
    for (unsigned int i = 0; i < sentenceVector.size(); i++){
        cerr << "i:0     1     2" << endl;
        for (unsigned int j = 0; j < sentenceVector.at(i).size(); j++)
            cerr << "[" << bitset<3>(sentenceVector.at(i).at(j)) << "] ";
        cerr << endl;
    }
}

/**
 * Fuegt die Pakete einer Binaerzahl wieder zu einer Binaerzahl zusammen.
 * Hierzu wird durch den Vektor iteriert, in welchem sich die Pakete
 * befinden. Dies werden dann mit einem Shift von 0, 3 oder 6 auf eine
 * neue 8-Bit Binaerzahl projeziert. Diese zusammengesetzte Binaerzahl wird
 * am Ende zurueckgegbene.
 * 
 * @param binaryVector   Ein Vektor mit den Teilpaketen, welche wieder zusammengefuegt werden sollen.
 * @return               Die zusammengefuegte 8-Bit Binaerzahl
 */
uint8_t mergeBitsets(vector<uint8_t> binaryVector){
    uint8_t mergedNum = 0b00000000;
    for (unsigned int i = 0; i < binaryVector.size(); i++)
        mergedNum |= (binaryVector.at(i) << (i * 3));
    return mergedNum;
}

/**
 * Sendet eine Anfrage an den anderen PC, ob eine Uebertragung gestartet werden kann.
 * Hierzu wird die Konstante REQ_SIGN an das andere Board gesendet und auf eine
 * Antwort gewartet. Insgesamt wird 40-mal 500ms gewartet bis die Anfrage ins
 * Timeout geht. Wenn während dieses Zeitraums der andere PC eine Bestaetigung
 * in Form eines ACK_SIGN sendet. Wird wahr zurueckgegeben, ansonsten wird am 
 * Ende der 40 Versuche ein false zurueckgegeben.
 * 
 * @param drv    Ein Objekt als Referenz auf eine Instanz des B15-Boards.
 * @return       Wahrheitswert, ob die Anfrage bestaetigt oder abgelehnt wurde.
 */
bool sendRequest(B15F &drv){
    drv.setRegister(&PORTA,sendConverting(REQ_SIGN));
    for(int i = 0; i < 40; i++){
        if(reveiceConverting(drv.getRegister(&PINA)) == ACK_SIGN){
            cerr << "[Comm]: >>Uebertragung akzeptiert<<" << endl;
            return true;
        }
        else{
            cerr << "[Comm]: Waiting for Acknowledge... (" << i << "/40)" << endl;
        }
        drv.delay_ms(500);
    }
    cerr << "[Comm]: Error - Request Timout" << endl;
    return false;
}

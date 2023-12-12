#include <iostream>
#include <b15f/b15f.h>
#include <vector>
#include <string>

using namespace std;

void dissectString(const std::string&, vector<char>&);

int main(){
	string sentence;
	if(std::getline(cin, sentence)) {
		vector<char> charVector;
		dissectString(sentence, charVector);
		
		cout << "String in Vektor zerlegt: " << endl;
		for(char character : charVector) {
			cout << character << endl;
			string s = "";
			/*for (int i = 0; i < 3; i++) {
				s += std::bitset<3>(binary.at(i)).to_string();
			}*/
		}
		//cout << endl;
	} else {
		cout << "[System]: Error" << endl;
	}
	
}

void dissectString(const string& sentence, vector<char>& charVector) {
	if(sentence.empty()) {
		cout << "falsche Eingabe" << endl;
	}
	charVector.assign(sentence.begin(), sentence.end());
}

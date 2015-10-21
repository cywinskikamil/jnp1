#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <boost/regex.hpp>
#include <algorithm>
#include <cfenv>
#include <cmath>
#include <iomanip>

#define mp make_pair
#define pb push_back

using std::vector;
using std::pair;
using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::string;
using std::make_pair;
using std::tuple;

typedef pair<string, double> sd;
typedef tuple<double, string, int> ds;
typedef vector<sd> vsd; // wektor do trzymania par (nazwa waluty, kwota) jeśli są różne nazwy kwot
typedef vector<dsi> vdsi; // wektor do trzymania par (suma donacji, nazwa darczyncy, nr donacji)
typedef pair<vsd, int> pvsdi;

enum inputType {CURRENCIES, DONATORS, REQUESTS, ERROR};

const int ERR = -1;
const int ROUNDTO = 3;

inputType readInput(string *input, inputType *readingPhase, boost::cmatch *result){
    static const string number_regex = "(([[:d:]]+)((,[[:d:]])?)(([[:d:]])?){2})"; //Regex wczytujący liczbę spełniająca warunki zadania
    static const string polish_letters_regex = "ąćęłńóśźżĄĆĘŁŃÓŚŹŻ";
    static const string currency_regex = "(\\s*)([A-Z]{3})(\\s+)" + number_regex + "(\\s*)"; //Regex wczytujący przelicznik waluty
    static const string donator_regex = "(\\s*)(([" + polish_letters_regex + "a-zA-Z]+)([" + polish_letters_regex + "a-zA-Z\\s]*))(\\s+)" + number_regex + "(\\s+)([A-Z]{3})(\\s*)"; //Regex dawcy
    static const string request_regex = "(\\s*)" + number_regex + "(\\s+)" + number_regex + "(\\s*)"; //Regex wczytujący zapytania
    
    static const boost::regex currency_expression(currency_regex);
    static const boost::regex donator_expression(donator_regex);
    static const boost::regex request_expression(request_regex);
    
    string inputString;
    getline(cin, inputString);
    *input = inputString;
    switch (*readingPhase){
        case CURRENCIES:
            if (boost::regex_match(inputString.c_str(), *result, currency_expression)){
                return CURRENCIES;
            } else if (boost::regex_match(inputString.c_str(), *result, donator_expression)){
                *readingPhase = DONATORS;
                return DONATORS;
            } else if (boost::regex_match(inputString.c_str(), *result, request_expression)){
                *readingPhase = REQUESTS;
                return REQUESTS;
            } else {
                return ERROR;
            }
        case DONATORS:
            if (boost::regex_match(inputString.c_str(), *result, donator_expression)){
                return DONATORS;
            } else if (boost::regex_match(inputString.c_str(), *result, request_expression)){
                *readingPhase = REQUESTS;
                return REQUESTS;
            } else {
                return ERROR;
            }
        case REQUESTS:
            if (boost::regex_match(inputString.c_str(), *result, request_expression))
                return REQUESTS;
            else
                return ERROR;
        default:
            return ERROR;
    }
}

void printError(const string input, const int lineCount){
    std::cerr<<"Error in line "<<lineCount<<":"<<input<<"\n";
}

double getValue(const boost::cmatch result, const int index){
    string str = (string) result[index];
    replace(str.begin(), str.end(), ',', '.');
    try {
        return stod(str);
    }
    catch (const std::out_of_range& oor) {
        return ERR;
    }
    catch (std::invalid_argument& ia) {
        return ERR;
    }
}

string doubleWithComma(double num){
    string str = std::to_string(num);
    replace(str.begin(), str.end(), '.', ',');
    size_t idx;
    for (size_t i = str.size() - 1; i >= 0; i--){
    	if ((i >= 3 && str[i-3] == ',') || str[i] != '0'){
    		idx = i;
    		break;
    	}
    }
    return str.substr(0, idx + 1);
}

bool doubleCheck(const double value){
    return (value != ERR);
}

bool checkUnderOverFlow(double num1, double num2){
    std::feclearexcept(FE_OVERFLOW);
    std::feclearexcept(FE_UNDERFLOW);
    num1 *= num2;
    return std::fetestexcept(FE_OVERFLOW) || std::fetestexcept(FE_UNDERFLOW);
}

double roundDouble(double num, const int decimalPlaces) {
	num *= 1.00001;
    double numFloor = floor(num);
    num = num - numFloor;
    int temp = (int)(num * pow(10, decimalPlaces + 1));
    
    if (temp % 10 < 5){
        return numFloor + (temp/10)/((double)pow(10, decimalPlaces));
    } else if (temp % 10 > 5){
        return numFloor + (temp/10 + 1)/((double)pow(10, decimalPlaces));
    } else {
        temp = temp/10;
        if (temp % 2 == 0)
            return numFloor + (temp)/((double)pow(10, decimalPlaces));
        else
            return numFloor + (temp + 1)/((double)pow(10, decimalPlaces));
    }
}

double countDonatesSingleDonator(map<string, double> *currencies, vsd *donates){
    double sum = 0.0;
    for (size_t i = 0; i < (*donates).size(); i++){
        string currencyId = (*donates)[i].first;
        sum += roundDouble((*donates)[i].second * (*currencies)[currencyId], ROUNDTO);      // ZAOKRAGLANIE?, OVERFLOW?
    }
    return sum;
}

void processAllDonates(map<string, double> *currencies, map<string, vsd> *donators,
                       vds *donatesInLocalCurrency){
    for (map<string, vsd>::iterator it = (*donators).begin(); it != (*donators).end(); it++){
        (*donatesInLocalCurrency).pb(mp(countDonatesSingleDonator(currencies, &it->second) , it->first));	//A TU JAK REFERENCJE?
    }
    stable_sort((*donatesInLocalCurrency).begin(), (*donatesInLocalCurrency).end());
}

bool myCmp(const pair<double, string> &a, const pair<double, string> &b){
    return a.first < b.first;
}

void showAllDonates(string name, map<string, vsd> *donators){
    vsd * donator = &(*donators)[name];
    for (size_t i = 0; i < (*donator).size(); i++){
        string currency = (*donator)[i].first;
        double amount = (*donator)[i].second;
        cout << "\"" << name << "\",\"" << doubleWithComma(amount) << "\"," << currency << endl;
    }
}

void processCurrencyData(const boost::cmatch result, const string input, const int lineCount,
                         map<string, double> *currencies){
    string currencyId;
    double exchange;
    currencyId = (string) result[2]; //na pozycji 2 znajduje sie ID podanej waluty
    exchange = getValue(result, 4); //na pozycji 4 znajduje sie kurs wymiany podanej waluty
    if (!doubleCheck(exchange) || (*currencies)[currencyId] != 0 || exchange <= 0){
        printError(input, lineCount);
        return;
    }
    (*currencies)[currencyId] = exchange;
}

void processDonatorData(const boost::cmatch result, const string input, const int lineCount,
                        map<string, double> *currencies, map<string, vsd > *donators){
    string donatorName, currencyId;
    double amount;
    donatorName = (string) result[2]; //na pozycji 2 znajduje się pełna nazwa dawcy
    amount = getValue(result, 6); //na pozycji 6 znajduje się kwota wpłacona przez dawcę
    currencyId = (string) result[13]; //na pozycji 13 znajduje się ID waluty, w której wpłacił dawca
    double currency = (*currencies)[currencyId];
    if (!doubleCheck(amount) || currency == 0 || checkUnderOverFlow(currency, amount)){    	
        printError(input, lineCount);
        return;
    }
    (*donators)[donatorName].pb(mp(currencyId, amount));
}

void processRequestData(const boost::cmatch result, const string input, const int lineCount,
                        map<string, vsd > *donators, vds *donatesInLocalCurrency){
    double firstAmount, secondAmount;
    firstAmount = getValue(result, 2); //na pozycji 2 znajduje się pierwsza kwota zapytania
    secondAmount = getValue(result, 9); //na poyzcji 9 znajduje się druga kwota zapytania
    if (!(doubleCheck(firstAmount) && doubleCheck(secondAmount) && firstAmount <= secondAmount)){
        printError(input, lineCount);
        return;
    }
    auto it = lower_bound((*donatesInLocalCurrency).begin(), (*donatesInLocalCurrency).end(),
                          mp(firstAmount, ""), myCmp);    
    while(it != (*donatesInLocalCurrency).end() && (*it).first <= secondAmount){
        showAllDonates((*it).second, donators);
        it++;
    }
}

int main(int argc, const char * argv[]) {
    bool notYetProcessed = true;
    map<string, double> currencies;
    map<string, vsd > donators;
    vds donatesInLocalCurrency;
    
    string input;
    inputType readingPhase = CURRENCIES;
    boost::cmatch result;
    int lineCount = 1;
    
    while(cin){
        switch(readInput(&input, &readingPhase, &result)){
            case CURRENCIES:
                processCurrencyData(result, input, lineCount, &currencies);
                break;
            case DONATORS:
                processDonatorData(result, input, lineCount, &currencies, &donators);
                break;
            case REQUESTS:
                if (notYetProcessed){
                    processAllDonates(&currencies, &donators, &donatesInLocalCurrency);
                    notYetProcessed = false;
                }
                processRequestData(result, input, lineCount, &donators, &donatesInLocalCurrency);
                break;
            default:
                if (cin)
                    printError(input, lineCount);
                break;
        }
        lineCount++;
    }
    return 0;
}
//KOLEJNOSC WPLAT


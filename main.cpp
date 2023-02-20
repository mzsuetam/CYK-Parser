#include <iostream>
#include <fstream>
#include <set>
#include <cstring>
#include <utility>
#include <csignal>
#include <vector>
#include <map>

class ModelException : public std::exception {
    std::string message;
public:
    explicit ModelException(std::string msg) : message(std::move(msg)) {}

    virtual const char* what () const noexcept {
        return message.c_str();
    }
};



class Grammar{
    std::set<char> terminals;
    std::set<char> nonTerminals;
    std::set< std::pair<char,std::string>> productions;
    char startingSymbol;

    static char checkTerminal(char c){
        if ( c < 97 || c > 122 ){
            std::string msg = "Grammar contains invalid terminal (";
            msg.push_back(c);
            msg.append( ")!");
            throw ModelException( msg);
        }
        return c;
    }

    static char checkNonTerminal(char c){
        if ( c < 65 || c > 90 ){
            std::string msg = "Grammar contains invalid non-terminal (";
            msg.push_back(c);
            msg.append( ")!");
            throw ModelException( msg);
        }
        return c;
    }

    std::pair<char, std::string> checkProduction(const std::string &str){
        size_t d = str.find('>');
        size_t l = str.length();
        if ( l>=2 && l<=4 && d==1 ){
            if ( nonTerminals.find( str.at(0) ) != nonTerminals.end() ){
                if ( ( l==2 )
                    || ( l==3 && terminals.find(str.at(2)) != terminals.end() )
                    || ( l==4 && nonTerminals.find( str.at(2) ) != nonTerminals.end()
                        && nonTerminals.find( str.at(3) ) != nonTerminals.end() )
                    ){
                    return {str.at(0), str.substr(2)};
                }
            }
        }

        throw ModelException("Grammar contains invalid production (" + str + ")!");
    }

    static char getFirstChar(std::string &str){
        if ( str.length() < 1 ) throw ModelException("Grammar file corrupted!");

        char val = str.at(0);
        str = ( str.length() == 1 ) ? "" : str.substr(2);
        return val;
    }

    static std::string getFirstString(std::string &str, char del=' '){
        size_t d = str.find(' ');
        if ( str.length() == 0 ){
            throw ModelException("Grammar file corrupted!");
        }

        std::string val = str.substr(0,d);
        str = ( str.length() == 1 || d == std::string::npos ) ? "" : str.substr(d+1);
        return val;
    }

    void readFile(const std::string &path){
        std::ifstream in;
        in.open(path);
        if(in.fail()) {
            throw ModelException("Grammar file corrupted!");
        }

        try{
            std::string line;

            // first line - terminals
            getline(in, line);
            while ( line.length() > 0 ){
                terminals.insert(checkTerminal(getFirstChar(line)));
            }

            // second line - non-terminals
            getline(in, line);
            while ( line.length() > 0 ){
                nonTerminals.insert(checkNonTerminal(getFirstChar(line)));
            }

            // third line - productions
            getline(in, line);
            while ( line.length() > 0 ){
                auto prod = checkProduction(getFirstString(line));
                productions.insert(prod);
            }

            // last line - starting symbol
            getline(in, line);
            if ( line.length() != 1 || nonTerminals.find( line.at(0) ) == nonTerminals.end() ) throw ModelException("Grammar starting symbol invalid!");
            startingSymbol = getFirstChar(line);

        }
        catch ( ... ) {
            throw;
        }
    }

public:
    Grammar(const std::string &path) {
        try{
            readFile(path);
        }
        catch( const std::exception &e ){
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    void print(){
        std::string T, N, P, S;
        for ( const auto &t : terminals ){
            T.append(&t).append(",");
        }
        T = T.substr(0, T.length()-1);
        for ( const auto &n : nonTerminals ){
            N.append(&n).append(",");
        }
        N = N.substr(0, N.length()-1);
        for ( const auto &p : productions ){
            P.append(&p.first).append(">").append(p.second).append(",");
        }
        P = P.substr(0, P.length()-1);
        S = startingSymbol;
        std::cout << "G = (T,N,P,S)"
        << std::endl << "\tT={" + T + "}"
        << std::endl << "\tN={" + N + "}"
        << std::endl << "\tP={" + P + "}"
        << std::endl << "\tS=" + S
        << std::endl;
    }

    static void printTab(std::vector< std::vector < std::set<char>* >* >* tab, const std::string &str){
        int n=str.length();
        for ( int i=0; i<n; i++ ) {
            std::cout << "\t" << str.substr(i,1) + "\t|";
        }
        std::cout << std::endl;
        for ( int i=0; i<n; i++ ){
            for ( int j=0; j<n; j++ ){
                std::cout << "\t";
                for ( const auto &c : *tab->at(j)->at(i) ){
                    std::cout << c;
                }
                if ( tab->at(j)->at(i)->empty() &&  i + j < n ) std::cout << '-';
                if ( i + j < n ) std::cout << "\t|";
            }
            std::cout << std::endl;
        }
    }

    bool checkIfElementOfLanguage(const std::string &str){
        // CYK algorithm

        for ( const auto &c : str ){
            if ( terminals.find(c) == terminals.end() ) return false;
        }

        auto tab = new std::vector< std::vector < std::set<char>* >* > ();
        for ( int i=0; i<str.length(); i++ ){
            tab->push_back(new std::vector<std::set<char>*>());
            for ( int j=0; j< str.length(); j++ ){
                tab->at(i)->push_back(new std::set<char>());
            }
        }

        for ( int i=0; i<str.length(); i++ ) {
            for ( const auto &p : productions ){
                if ( p.second.length() == 1 && p.second.at(0) == str.at(i) ){
                    tab->at(i)->at(0)->insert(p.first);
                }
            }
        }

        int n = str.length();
        for ( int i=2; i<=n; i++ ){
            for ( int j=1; j<=n-i+1; j++ ){
                for ( int k=1; k<=i-1; k++){
//                    std::cout << i << j << k << " | j k " << j-1 << k-1 << " | j+k i-k " << j+k-1 << i-k-1 << std::endl;
                    for ( const auto &p : productions ){
                        if ( p.second.length() == 2
                            && tab->at(j-1)->at(k-1)->find(p.second.at(0)) != tab->at(j-1)->at(k-1)->end()
                            && tab->at(j+k-1)->at(i-k-1)->find(p.second.at(1)) != tab->at(j+k-1)->at(i-k-1)->end()
                            ){
                            tab->at(j-1)->at(i-1)->insert(p.first);
                        }
                    }
                }
            }
        }
        printTab(tab, str);
        if ( tab->at(0)->at(n-1)->find(startingSymbol) != tab->at(0)->at(n-1)->end() ){
            return true;
        }
        return false;
    }
};

void sigHandler(int s){
//    printf("Caught signal %d\n",s);
    exit(1);
}

int main(int argc, char *argv[]) {

    std::string path = "./grammar.txt";

    for ( int i=0; i<argc; i++ ){
        if ( strcmp(argv[i], "-path") == 0 ){
            path = argv[++i];
        }
    }

    auto grammar = new Grammar(path);
    grammar->print();

    struct sigaction sigIntHandler{};
    sigIntHandler.sa_handler = sigHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, nullptr);

    std::cout << "Type !q or press CTRL-C to exit." << std::endl;

//    std::cout << grammar->checkIfElementOfLanguage("aabbb") << std::endl;

    std::string word;
    do {
        std::cout << "Word to be checked: ";
        std::cin >> word;

        auto belongs = grammar->checkIfElementOfLanguage(word);
        std::cout << "'" << word << "' " << ( belongs ? "BELONGS" : "DOES NOT BELONG" )
        << " to the language" << std::endl;

    } while ( word != "!q" );

    return 0;
}

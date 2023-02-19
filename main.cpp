#include <iostream>
#include <fstream>
#include <set>
#include <cstring>
#include <utility>

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
    std::set< std::string > productions;
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

    std::string checkProduction(const std::string &str){
        size_t d = str.find('>');
        size_t l = str.length();
        if ( l>=2 && l<=4 && d==1 ){
            if ( nonTerminals.find( str.at(0) ) != nonTerminals.end() ){
                if ( ( l==2 )
                    || ( l==3 && terminals.find(str.at(2)) != terminals.end() )
                    || ( l==4 && nonTerminals.find( str.at(2) ) != nonTerminals.end()
                        && nonTerminals.find( str.at(3) ) != nonTerminals.end() )
                    ){
                    return str;
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
                productions.insert(checkProduction(getFirstString(line)));
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
            P.append(p).append(",");
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
};

int main(int argc, char *argv[]) {

    std::string path = "./grammar.txt";

    for ( int i=0; i<argc; i++ ){
        if ( strcmp(argv[i], "-path") == 0 ){
            path = argv[++i];
        }
    }

    auto grammar = new Grammar(path);
    grammar->print();




    return 0;
}

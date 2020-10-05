
class Loader{
	private:
    	uint64_t prevAddy;
        bool loaded;        //set to true if a file is successfully loaded into memory
        std::ifstream inf;  //input file handle
        bool badFile(char argv[]);
        bool hasErrors(std::string ld);
        bool loadLine(std::string ld); // change to boolean?
        std::string getAddy(std::string ld);
        bool hasAddyErrors(std::string ld);
        uint64_t convert(std::string ld, int start, int end);
        int findSpace(std::string ld, int start, int end);
    	bool hasAddy(std::string ld);
    	bool isHexdigit(char digit);
    	std::string getData(std::string ld);
    	bool hasDataErrors(std::string ld);
    	bool isSpace(int start, int end, std::string ld);
        bool checkBlank(std::string line);
		bool checkSpecial(std::string line);
		bool checkValBytes(std::string line);
		bool checkHex(std::string line, int start, int end);
		bool hasNewAddress(std::string ld);
		bool insideMemory(std::string ld);
    public:
        Loader(int argc, char * argv[]);
        bool isLoaded();
};

/**
 * Names: Jonah Ross, Zack Beane
 * Team: 1
*/
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <ctype.h>
#include "Loader.h"
#include "Memory.h"
#include "Tools.h"
//first column in file is assumed to be 0
#define ADDRBEGIN 2   //starting column of 3 digit hex address 
#define ADDREND 4     //ending column of 3 digit hext address
#define DATABEGIN 7   //starting column of data bytes
#define COMMENT 28    //location of the '|' character 

/**
 * Loader constructor
 * Opens the .yo file named in the command line arguments, reads the contents of the file
 * line by line and loads the program into memory.  If no file is given or the file doesn't
 * exist or the file doesn't end with a .yo extension or the .yo file contains errors then
 * loaded is set to false.  Otherwise loaded is set to true.
 *
 * @param argc is the number of command line arguments passed to the main; should
 *        be 2
 * @param argv[0] is the name of the executable
 *        argv[1] is the name of the .yo file
 */
Loader::Loader(int argc, char * argv[]){
	prevAddy = 0;
    loaded = false;
	std::string ld;
	int ldnum = 0;
	bool error = false;
	if(!badFile(argv[1])){
		//You are not loding the file?
		//to be fair i was using the pseudo that she gave em
		while(inf.peek() != EOF && !error){
            getline(inf, ld);
            ldnum++;
    		if(!hasErrors(ld)){
    			if(ld.at(0) == '0' && ld.at(DATABEGIN) != ' '){
        			if(loadLine(ld)){
        				error = true;
        			}
       	 		}
       	 	}else{
   	 			error = true;
  	  			std::cout << "Error on line " << std::dec << ldnum
    			<< ": " << ld << std::endl;
    		}
   	 	}
        loaded = !error;
    }
}
            


/**
 * isLoaded
 * returns the value of the loaded data member; loaded is set by the constructor
 *
 * @return value of loaded (true or false)
 */
bool Loader::isLoaded(){
	return loaded;
}


//You'll need to add more helper methods to this file.  Don't put all of your code in the
//Loader constructor.  When you add a method here, add the prototype to Loader.h in the private
//section.

//----------------------------------------------badFile--------------------------

bool Loader::badFile(char argv[]){
    int size = strlen(argv);
    if((size > 3) && (argv[size - 3] == '.') && (argv[size - 2] == 'y') && (argv[size - 1] == 'o')) {
        inf.open(argv, std::ifstream::in);
        if(inf.good()){
            return false;
        }
    }
    return true;         
}

//-----------------------------------------------convert----------------------------------
uint64_t Loader::convert(std::string ld, int start, int end){
	std::string str;
	str = ld.substr(start, end-start+1);
    return std::stoul(str, nullptr, 16);
}

//-----------------------------------------------findSpace--------------------------
int Loader::findSpace(std::string ld, int start, int end){
    int space = start;
    while (ld.at(space) != 0x20 && space <= end){
        space++;
    }
    return space;
}

//-----------------------------------------------LoadLine--------------------------
bool Loader::loadLine(std::string ld){    
    Memory * mem = Memory::getInstance();
    bool error = false;
    
    uint64_t addy = convert(ld, ADDRBEGIN, ADDREND);
    if(addy >= 0){  //addy > prevAddy || addy == 0
     	for(int i = DATABEGIN; ld.at(i) != ' '; i += 2){
    	   	uint64_t data = convert(ld, i, i+1);
   	    	mem->putByte(data, addy, error);
   	    	prevAddy=addy;
			addy+=1;
  	  	}
    }else{
    	error = true;
    }
    return error;
}

//-----------------------------------------------hasErrors--------------------------
bool Loader::hasErrors(std::string ld){
    if (checkBlank(ld)){
    	if(!checkSpecial(ld) && !hasAddyErrors(ld) && !hasDataErrors(ld) 
    		&& (convert(ld, ADDRBEGIN, ADDREND) >= prevAddy) //make sure addy is not a duplicate
    		&& insideMemory(ld)){
        	return false;
    	}else{
    		return true;
    	}
    }
    else if(ld[COMMENT] != '|'){
     	return true;
    }
    return false;
}

//---------------------------------------------insideMemory-------------------------
bool Loader::insideMemory(std::string ld){
	//if()
	return true;
}


//------------------------------------------------checkBlank------------------------
bool Loader::checkBlank(std::string line){
    for(int i = 0; i < COMMENT; i++){
        if(line[i] != ' '){
            return true;
        }
    }
    return false;
}

//------------------------------------------------isHexDigit--------------------------
bool Loader::checkHex(std::string line, int start, int end){
    for(int i = start; i <= end; i++){
        char hex = line.at(i);
        if (!((hex >= 0x30 && hex <= 0x39)
            || (hex >= 0x41 && hex <= 0x46)
            || (hex >= 0x61 && hex <= 0x66)
            || (hex == 0x20))) {
            return true;
        }
    }
	return false;
}

//------------------------------------------------checkSpecial------------------------
bool Loader::checkSpecial(std::string line){
	if(line.at(5) != ':'||
    	line.at(6) != ' ' ||
        line.at(COMMENT) != '|'){
		return true;
    }
    return false;
}

//-----------------------------------------------hasAddyErrors--------------------------
bool Loader::hasAddyErrors(std::string ld){
	if(ld[0] == '0' && ld[1] == 'x'){
		return checkHex(ld, ADDRBEGIN, ADDREND);
    }
    return true;
}


bool Loader::checkValBytes(std::string ld){
    int charCount = 0;
    bool hasSpace = false;
    for(int i = DATABEGIN; i < COMMENT; i++){
        if(hasSpace && ld[i] != ' '){
            return true;
        }
        else if(ld[i] == ' '){
            hasSpace = true;
        }else{
        	charCount++;
        }
    }
    if(charCount > 0 && (charCount % 2) != 0){
        return true;
    }
    uint64_t addy = convert(ld, ADDRBEGIN, ADDREND);
	if(charCount/2 + addy > MEMSIZE){
		return true;
	}
    return false;
}


std::string Loader::getData(std::string ld){
    std::string data;
    for (int i = DATABEGIN; i < COMMENT; i++){
        if(!isSpace(i, i, ld)){
        	data += ld[i];
        }
    }
	return data;
}

bool Loader::hasDataErrors(std::string ld){
    if(!checkValBytes(ld))
    {
    	return checkHex(ld, DATABEGIN, COMMENT-1);
    }
	return true;
}

bool Loader::isSpace(int start, int end, std::string ld){
	for(int i = start; i <= end; i++){
    	if(ld[i] == ' '){
        	return true;
        }
    }
    return false;
}


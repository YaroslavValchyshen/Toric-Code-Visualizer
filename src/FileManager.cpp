#include "FileManager.hpp"

std::string FileManager::readFile(std::string fileName){
    std::string content;
    std::string currentLine;
    std::ifstream file(fileName);
    
    while(std::getline(file, currentLine)){
        content += currentLine + "\n";
    }
    file.close();
    
    return content;
    
}
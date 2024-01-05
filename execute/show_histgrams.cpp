#include <iostream>
#include <fstream>
#include "../include/ShowHistgrams.hpp"

int main( int argc, char *argv[] ) {  
  std::cout << std::endl << "============================================================ run : Show Histgrams" << std::endl << std::endl;  
  std::string filePath = argv[1];
  std::string filePath2 = argv[2];
  ShowHistgrams( filePath, filePath2 );
  
}

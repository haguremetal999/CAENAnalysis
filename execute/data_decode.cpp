#include <iostream>
#include "../include/ReadTxtData.hpp"

int main( int argc, char *argv[] ) {
  std::cout << std::endl << "============================================================ run : Data_Decode" << std::endl << std::endl;
  
  std::string filePath = argv[1];
  int fileSize = atoi ( argv[2] );
  ReadTxtData( filePath, fileSize );

}

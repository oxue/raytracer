#include <iostream>
#include "scene_lua.hpp"
#include <string.h>

bool SHORTCUT = false;

int main(int argc, char** argv)
{
  std::string filename = "Assets/simple.lua";
  if (argc >= 2) {
    filename = argv[1];
  }
  if(argc >=3){
  	if(strcmp(argv[1], "-acc") == 0){
  		std::cout<< "USING ACCELERATION"<<std::endl;
  		SHORTCUT = true;
  	}
  	filename = argv[2];
  }

  if (!run_lua(filename)) {
    std::cerr << "Could not open " << filename << std::endl;
    return 1;
  }
}

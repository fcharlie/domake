/**
*
**/
#include <iostream>
#include <string>

int ParseArgv(int Argc,char **Argv)
{
	if(Argc<1){
		std::cout<<"Usage: "<<std::endl;
	}else{
		std::cout<<"Usage: "<<Argv[1]<<std::endl;
	}
	return 0;
}

int main(int argc, char **argv) {
  ////
  
  std::cout<<"Helloworld"<<std::endl;
  return ParseArgv(argc,argv);
}

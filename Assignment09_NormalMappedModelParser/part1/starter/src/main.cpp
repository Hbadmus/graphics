// Support Code written by Michael D. Shah
// Last Updated: 6/10/21
// Please do not redistribute without asking permission.

// Functionality that we created
#include "SDLGraphicsProgram.hpp"

#include <iostream>

int main(int argc, char** argv){

	std::cout << "Please remember:\n For this starter code you only need to work in the shader. That also means, once you compile your .cpp files, you need only run your ./lab or ./lab.exe once, because every time your program runs it will recompile the shaders which you are making changes to. So save yourself some time :)\n\n" << std::endl;

	// Create an instance of an object for a SDLGraphicsProgram
	SDLGraphicsProgram mySDLGraphicsProgram(1280,720);
	// Run our program forever
	mySDLGraphicsProgram.Loop();
	// When our program ends, it will exit scope, the
	// destructor will then be called and clean up the program.
	return 0;
}

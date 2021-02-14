Windows:
	@mkdir -p "Windows"
	$(MAKE) -f make_impl BUILD_DIR=Windows CXX="x86_64-w64-mingw32-g++-posix" LIBS="mingw32 opengl32 gdi32 glew32" LDFLAGS_IN="-static" CXXFLAGS_IN="-g3 -municode" INCLUDE_DIR="/home/caroline/win32_includes/include/" LIBRARY_DIR=/home/caroline/win32_includes/lib/ 
	  
Debug: 
	@mkdir -p "Debug"
	$(MAKE) -f make_impl BUILD_DIR=Debug CXX="g++-10" LIBS="asan ubsan  freetype GLEW GL X11" CXXFLAGS_IN="-Wno-unused-parameter -fsanitize=undefined -fsanitize=address -g3 -Wall -Wextra" LIBRARY_DIR=/usr/local/lib
	  
Linux: 
	@mkdir -p "Linux"
	$(MAKE) -f make_impl BUILD_DIR=Linux CXX="g++-8" LIBS="freetype GLEW GL X11" CXXFLAGS_IN="-O3"

Test:
	@mkdir -p "Test"
	$(MAKE) -f make_impl BUILD_DIR=Test CXX="g++-10" LIBS="freetype GLEW GL X11" CXXFLAGS_IN="--coverage" LIBRARY_DIR=/usr/local/lib

clean: 
	rm -rf Debug/
	rm -rf Debug_rpggame
	rm -rf Test/
	rm -rf Test_rpggame

.PHONY: Windows Debug Linux Test clean
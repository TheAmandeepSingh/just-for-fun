CXX = g++-14
CXXFLAGS = -fopenmp -I/usr/local/include -I/opt/homebrew/include
LDFLAGS = -L/usr/local/lib -L/opt/homebrew/lib -lpng -fopenmp

heart_fractal: heart_fractal.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f heart_fractal
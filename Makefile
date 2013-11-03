APP=pdfrenamer
CXXFLAGS=-g3 -O0 -lpoppler-cpp -Wall -pedantic

all: $(APP)

$(APP): main.cc
	$(CXX) -o $@ $^ $(CXXFLAGS)

test: $(APP)
	./$(APP) test.pdf

clean:
	$(RM) $(APP)

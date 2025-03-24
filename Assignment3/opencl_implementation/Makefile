CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
LDFLAGS = -lOpenCL

# Windows-specific paths (adjust if needed)
OPENCL_INC = C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.8/include
OPENCL_LIB = C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.8/lib/x64

# Include and library paths
INCLUDES = -I$(OPENCL_INC) -I.
LIBS = -L$(OPENCL_LIB)

TARGET = edge_detector
SRC = main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@ $(LIBS) $(LDFLAGS)

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean

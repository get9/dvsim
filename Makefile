CXX = g++

# Final binary
BIN = mycode
# Put all auto generated stuff to this build dir.
BUILD_DIR = ./build
INCLUD_DIR = include

CXX_FLAGS = -I$(INCLUD_DIR) -std=gnu++0x -Wfatal-errors -Wall -Wextra -Wconversion -Wshadow -pthread
LD_FLAGS = -pthread

# List of all .cpp source files.
CPPS = $(wildcard src/*.cpp)

# All .o files go to build dir.
OBJ = $(CPPS:%.cpp=$(BUILD_DIR)/%.o)
# Gcc/Clang will create these .d files containing dependencies.
DEP = $(OBJ:%.o=%.d)

# Default target named after the binary.
$(BIN) : $(BUILD_DIR)/$(BIN)

# Actual target of the binary - depends on all .o files.
$(BUILD_DIR)/$(BIN) : $(OBJ)
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) $(LD_FLAGS) $^ -o $@

# Include all .d files
-include $(DEP)

# Build target for every single object file.
# The potential dependency on header files is covered
# by calling `-include $(DEP)`.
$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@

debug:  CXX_FLAGS += -g -DDEBUG

debug: $(BIN)

.PHONY : clean
clean :
	-rm $(BUILD_DIR)/$(BIN) $(OBJ) $(DEP)

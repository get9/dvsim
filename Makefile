CXX = clang++

# Final binary
BIN = mycode
# Put all auto generated stuff to this build dir.
BUILD_DIR = ./build
INCLUD_DIR = include

CXX_FLAGS = -I$(INCLUD_DIR) -std=c++11 -Wfatal-errors -Wall -Wextra -Wpedantic -Wconversion -Wshadow

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
	# Create build directories - same structure as sources.
	mkdir -p $(@D)
	# Just link all the object files.
	$(CXX) $(CXX_FLAGS) $^ -o $@

# Include all .d files
-include $(DEP)

# Build target for every single object file.
# The potential dependency on header files is covered
# by calling `-include $(DEP)`.
$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
	# The -MMD flags additionaly creates a .d file with
	# the same name as the .o file.
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@

debug: CXX_FLAGS += -g -DDEBUG

debug: $(BIN)

.PHONY : clean
clean :
	# This should remove all generated files.
	-rm $(BUILD_DIR)/$(BIN) $(OBJ) $(DEP)

NAME        = privesc

CXX         = g++
CXXFLAGS    = -Wall -Wextra -Werror -Iinclude

SRC_DIR     = src
OBJ_DIR     = build/obj
BIN_DIR     = build/bin
LIB_DIR     = build

SIGNTOOL    = signtool
SIGN_FLAGS  = sign /a /fd SHA256 /tr http://timestamp.digicert.com /td SHA256

SRC = \
	src/core/privesc.cpp \
	src/fodhelper/fodhelper.cpp \
	src/other/isadmin.cpp \
	src/fodhelper/regedit.cpp \
	src/runas/elevate_runas.cpp

OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

LIB = $(LIB_DIR)/lib$(NAME).a

EXAMPLES = src/examples/basic.cpp
BINS = $(EXAMPLES:src/examples/%.cpp=$(BIN_DIR)/%.exe)

# ----------------------------------------

all: $(LIB) examples sign

# ----------------------------------------

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"
	@if not exist "$(OBJ_DIR)/core" mkdir "$(OBJ_DIR)/core"
	@if not exist "$(OBJ_DIR)/fodhelper" mkdir "$(OBJ_DIR)/fodhelper"
	@if not exist "$(OBJ_DIR)/runas" mkdir "$(OBJ_DIR)/runas"
	@if not exist "$(OBJ_DIR)/other" mkdir "$(OBJ_DIR)/other"
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ----------------------------------------

$(LIB): $(OBJ)
	@if not exist "$(LIB_DIR)" mkdir "$(LIB_DIR)"
	ar rcs $@ $^

# ----------------------------------------

$(BIN_DIR)/%.exe: src/examples/%.cpp $(LIB)
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -l$(NAME) -o $@

examples: $(BINS)

# ----------------------------------------

sign:
	@echo Checking signtool...
	@where $(SIGNTOOL) >nul 2>nul && ( \
		echo Signing binaries... && \
		for %%f in ($(BINS)) do @$(SIGNTOOL) $(SIGN_FLAGS) %%f \
	) || ( \
		echo signtool not found, skipping signing \
	)

# ----------------------------------------

clean:
	@if exist build rmdir /s /q build

fclean: clean

re: fclean all

# ----------------------------------------

.PHONY: all clean fclean re examples sign
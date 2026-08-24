NAME        = privesc

CXX         = g++
CXXFLAGS    = -Wall -Wextra -Werror -Iinclude -s -O2
LDLIBS      = -lole32

SRC_DIR     = src
OBJ_DIR     = build/obj
BIN_DIR     = build/bin
LIB_DIR     = build

SIGNTOOL    = signtool
SIGN_FLAGS  = sign /a /fd SHA256 /tr http://timestamp.digicert.com /td SHA256

SRC = \
	src/core/privesc.cpp \
	src/cmstplua/cmstplua.cpp \
	src/ifileoperation/ifileoperation.cpp \
	src/cddefaults/cddefaults.cpp \
	src/fodhelper/fodhelper.cpp \
	src/other/isadmin.cpp \
	src/other/encryption.cpp \
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
	@mkdir -p $(OBJ_DIR)/core $(OBJ_DIR)/cmstplua $(OBJ_DIR)/cddefaults $(OBJ_DIR)/ifileoperation $(OBJ_DIR)/fodhelper $(OBJ_DIR)/runas $(OBJ_DIR)/other
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ----------------------------------------

$(LIB): $(OBJ)
	@mkdir -p $(LIB_DIR)
	ar rcs $@ $^

# ----------------------------------------

$(BIN_DIR)/%.exe: src/examples/%.cpp $(LIB)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -l$(NAME) $(LDLIBS) -o $@

examples: $(BINS)

# ----------------------------------------

sign:
	@if command -v $(SIGNTOOL) >/dev/null 2>&1; then \
		echo Signing binaries...; \
		for f in $(BINS); do $(SIGNTOOL) $(SIGN_FLAGS) $$f; done; \
	else \
		echo signtool not found, skipping signing; \
	fi

# ----------------------------------------

clean:
	@rm -rf build

fclean: clean

re: fclean all

# ----------------------------------------

.PHONY: all clean fclean re examples sign

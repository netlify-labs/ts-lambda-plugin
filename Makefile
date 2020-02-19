CXX ?= g++

CXXFLAGS += --std=c++17 -fPIC

LDFLAGS += -ltscppapi
LDFLAGS += -laws-cpp-sdk-lambda
LDFLAGS += -shared

SRC := $(shell ls -1 src/*.cc | sort)
OBJS := $(patsubst %.cc, %.o, $(SRC))
LIB ?= ts-lambda-plugin.so

all: release
release: CXXFLAGS += -O2 -ggdb3 -DNDEBUG
release: $(LIB)

$(LIB): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@;

$(OBJS) : %.o : %.cc

clean:
	@rm -fv *.d *.lo $(LIB) $(LIB_DEBUG) $(OBJS) $(OBJS_DEBUG)

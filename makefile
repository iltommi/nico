BUILD_DIR ?= build

CXX ?= g++-8
PYCONFIG ?= python-config
EXEC = main

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)
DEPS := $(SRCS:.cpp=.d)

CXXFLAGS += -std=c++11 $(shell $(PYCONFIG) --includes)
LDFLAGS += $(shell $(PYCONFIG) --ldflags)

default: run

clean:
	rm -rf $(DEPS) $(OBJS) $(EXEC)
	
%.d: %.cpp
	@echo "Dependencies for $<"
	@$(CXX) $(CXXFLAGS) -MF"$@" -MG -MM -MP -MT"$@ $(@:.d=.o)" $<

%.o : %.cpp
	@echo "Compile for $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(EXEC): $(OBJS)
	@echo "Linking $@"
	@$(CXX) $(OBJS) -o $@ $(LDFLAGS) 

run: $(EXEC)
	./main test.py

-include $(DEPS)


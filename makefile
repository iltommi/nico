BUILD_DIR ?= build

CXX ?= g++-6

EXEC = main

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)
DEPS := $(SRCS:.cpp=.d)

CXXFLAGS += -std=c++11 $(shell python-config --includes)
LDFLAGS += $(shell python-config --ldflags)

default: $(EXEC)

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

-include $(DEPS)


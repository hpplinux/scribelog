EXECUTABLE :=scribe 

LIBS := lce pthread
NCE_PATH = /home/starjiang/lce/lce/
INC_PATH := $(NCE_PATH)/include
LIB_PATH := ./ $(NCE_PATH)/lib/  /usr/lib/

USER_MARCOS :=  NDEBUG
CFLAGS = -g -O -Wall -Wno-deprecated
CC = g++ 


SOURCE := $(wildcard *.cpp)  
OBJS := $(patsubst %.cpp,%.o,$(SOURCE))
  
%.o:%.cpp
	$(CC) $(CFLAGS) $(addprefix -D,$(USER_MARCOS)) $(addprefix -I,$(INC_PATH)) -c $< -o $@

$(EXECUTABLE): $(OBJS)
	$(CC) $(CFLAGS) $(addprefix -L,$(LIB_PATH))  $(addprefix -I,$(INC_PATH)) -o $(EXECUTABLE) $(OBJS) $(addprefix -l,$(LIBS)) 

clean :  
	rm -rf *.d *.o *.lo  $(EXECUTABLE)  
	 
install:

	


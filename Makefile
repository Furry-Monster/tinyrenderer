CC := gcc                                  
CFLAGS := -Wall -Wextra -pedantic -std=c99 
DEBUGFLAGS := -g                           
TARGET := tinyrenderer                         
SRC := tinyrenderer.c                          
OBJ := $(SRC:.c=.o)                        

# Default target                                      
.PHONY: all                                           
all: $(TARGET)                                        

# Build the executable                                
$(TARGET): $(OBJ)                                     
	$(CC) $(OBJ) -o $(TARGET)                           

# Compile source files to object files                
%.o: %.c                                              
	$(CC) $(CFLAGS) -c $< -o $@                         

# Debug build                                         
.PHONY: debug                                         
debug: CFLAGS += $(DEBUGFLAGS)                        
debug: clean all                                      

# Clean the build                                     
.PHONY: clean                                         
clean:                                                
	rm -f $(TARGET) $(OBJ)                              

# Help target                                         
.PHONY: help                                          
help:                                                 
	@echo "Available targets:"                          
	@echo "  all      : Build the executable (default)" 
	@echo "  debug    : Build with debug information"   
	@echo "  clean    : Remove build artifacts"         
	@echo "  install  : Install to /usr/local/bin"      
	@echo "  uninstall: Remove from /usr/local/bin"     
	@echo "  help     : Show this help message"         

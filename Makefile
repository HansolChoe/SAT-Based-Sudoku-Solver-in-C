CC = gcc

TARGET = sudoku
OBJS = sudoku.o

all : $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET) output.txt minissat.out sudoku.out

new :
	$(MAKE) clean
	$(MAKE)

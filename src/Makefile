BIN=../bin
FILES=parser.o fileoper.o lex.o xlog.o

O_FILES=$(addprefix $(BIN)/, $(FILES))
OUT_FILE=x.out

$(BIN)/$(OUT_FILE):$(O_FILES)
	gcc  $^ -o $@ -g

compile:$(O_FILES)

test:
	$(BIN)/$(OUT_FILE) a.xs 

$(BIN)/%.o:%.c
	gcc -c $< -o $@ -Wall -g

clean:
	rm -rf $(BIN)/*

debug:
	gdb $(BIN)/$(OUT_FILE)


tag:
	rm -rf tags
	ctags *


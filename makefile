bs:
	cc -Wall -g t.h t.c m.c m.h bst.h bst.c cell.h cell.c stdlib.h stdlib.c parse.h parse.c stdlib2.h stdlib2.c eval.h eval.c bs.c -o bs

clean:
	rm -f bs.exe

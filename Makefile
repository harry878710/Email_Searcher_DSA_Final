main: ./src/main.c ./src/api.h
	gcc ./src/main.c -o ./bin/main -O3 -std=c11 -w

validator/validator: validator/validator.cpp
	g++ validator/validator.cpp -o validator/validator -O3

.PHONY=run
run: main validator/validator
	./bin/main < testdata/test.in | validator/validator

.PHONY=print
print: main
	./bin/main < testdata/test.in

.PHONY=write
write: main validator/validator
	./bin/main < testdata/test.in > testdata/write.out
	
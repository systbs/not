compile the shared libraries as follows:
linux:
```sh
gcc -c -fPIC shared.c -o shared.o
gcc -c -fPIC ../not.c -o not.o
gcc -shared -o shared.so not.o shared.o -lgmp -ljansson
```

windows:
```sh
gcc -c -fPIC shared.c -o shared.o
gcc -c -fPIC ../not.c -o not.o
gcc -shared -o shared.dll not.o shared.o -lgmp -ljansson
```

macOs:
```sh
gcc -c -fPIC shared.c -o shared.o
gcc -c -fPIC ../not.c -o not.o
gcc -dynamiclib -o shared.dylib not.o shared.o -lgmp -ljansson
```
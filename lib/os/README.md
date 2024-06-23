compile the shared libraries as follows:
linux:
```sh
gcc -shared -o shared.so shared.c -fPIC -lgmp
```

windows:
```sh
gcc -shared -o shared.dll shared.c -fPIC -lgmp

macOs:
gcc -dynamiclib -o shared.dylib shared.c

```
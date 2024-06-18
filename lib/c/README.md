compile the shared libraries as follows:
```sh
gcc -shared -o shared.so shared.c -fPIC -lgmp
```
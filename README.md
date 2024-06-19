# <img src="assets/logo-base.png" alt="not logo" height="64"> The Not Programming Language

## Build Instructions
On Ubuntu:
```sh
chmod +x install_dependencies.sh
./install_dependencies.sh

make
make test
```

On Windows:
```sh
pacman -S mingw-w64-x86_64-jansson
pacman -S mingw-w64-x86_64-gmp
pacman -S mingw-w64-x86_64-libffi
pacman -S mingw-w64-x86_64-mpfr

make
make test
```

## 'Not' Needs You
Open source software is made better when users can easily contribute code and documentation to fix bugs and add features. 'Not' strongly encourages community involvement in improving the software.

## Sample Code
```not
// import
using param2: param from "~/lib/types/primary.not";

// on windows use "~/lib/c/shared_win.json"
using println, add from "~/lib/c/shared.json";

// convert string to float
var b1 = float ("12e-1");

// convert string to int
var b2 = int ("12345");

// round
var b3 = float (1.3451, 2);

var b4 = add(1, 2);

println("hello world" + " " + b1 + " " + b2 + " " + b3 + " " + param2 + " " + b4);

// types

// int type
var a1 = 1;

// float type
var a2 = 1.1;

// char type
var a3 = 'a';

// string type
var a4 = "simple text";

// tuple/array
var a5 = [1, 2, 3, 4, 5];

// object
var a6 = {a:1, b:2, c:3, d:4, e:5};

// class definition
class B
{
	// static property
	export static b = 1000000;

	// property
	export c = 0;

	// constructor
	export fun constructor(p1:int)
	{
		c = p1;
	}
}

// inheritance & generic types
class A<T> extends (b1:B)
{
	export static a = 1000000;

	export b:T = 10;

	export fun constructor(p1:int)
	{
		b1.constructor(1000000);
		b = b * p1;
	}

	// methods
	export fun Get() {
		return b;
	}

	// override operators
	export fun [](val:int, step:int = 1)
	{
		return b/val;
	}

	// override operators
	export fun * (p1:int)
	{
		return b * p1;
	}
}

// lambda functions
var fun1 = fun (p1:int) {
	return p1 * 1000;
};

// new class
var f2 = A<int>(100000);
var f3 = f2 * 2;

// typeof|instanceof
var f4 = typeof f2;
var f5 = f2 instanceof A<int>;

// access to item
var f6 = f4[2];

// slice -> [start, stop, step]
var f7 = a5[1, 2]; 
var f8 = a5[1, -1, 1];
println(string (f8));

export var sum = 0;

// try/catch
try {

	// for loop definition
	for loop1 (var a = 0;a < 1000000;a += 1)
	{
		sum += a;

		// if
		if (a > 500000)
		{
			throw "simple throw";
			continue loop1;
		}
	}
}
catch(ex)
{
	println(string(ex));
}

// forin loop
for loop1 (var key, value in ["hello", "world", 123])
{
	println(string(value));
}

var a7 = {a:1};

a7.Remove(key = "a");
a7.Add("b", 123);

println(string(a7));

var a8 = [0];
a8.Insert(12, 10).Remove(-2);

println(string(a8));

```

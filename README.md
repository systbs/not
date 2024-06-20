# <table style="border: 0px solid #ffffff;border-collapse: collapse;"><tr style="vertical-align: middle;"><th><img src="assets/logo-base.png" alt="not logo" height="64"></th><th>The Not Programming Language</th></tr></table> 

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
// import forigen function: on windows use "~/lib/c/shared_win.json"
using println, add from "~/lib/c/shared.json";

// import, if use windows import "~/lib/os/essential_win.not";
using print from "~/lib/os/essential.not";

// convert string to float
var b1 = float ("12e-1");

// convert string to int
var b2 = int ("12345");

// round
var b3 = float (1.3451, 2);

print("hello world" + " " + b1 + " " + b2 + " " + b3 + " ");

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

	export fun ToString()
	{
		return "class to string";
	}
}

// lambda functions
// or: fun (p1:int) -> p1 * 1000;
var fun1 = fun fun2 (p1:int) {
	if (p1 > 0)
	{
		print("f = " + p1);
		return fun2(p1 - 1);
	}
	print("f2 = " + p1);
	return p1 * 1000;
};

var f1 = fun1(100);
print("f1 = " + f1);

// new class
var f2 = A<int>(100000);
var f3 = f2 * 2;
print(f2);

// typeof|instanceof
var f4 = typeof f2;
var f5 = f2 instanceof A<int>;

// access to item
var f6 = f4[2];

// slice -> [start, stop, step]
var f7 = a5[1, 2]; 
var f8 = a5[1, -1, 1];
print(f8);

// try/catch
var sum = 0;
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
	print(ex);
}

// forin loop
for loop1 (var key, value in ["hello", "world", 123])
{
	print(value);
}

// object methods
var a7 = {a:1};

a7.Remove(key = "a");
a7.Add("b", 123);

print(a7);

// tuple/sequence methods
var a8 = [0];

a8.Set(1, 10);
a8.Append(15);
a8.Insert(4, 15);
a8.Remove(2);

print(a8);
print("count tuple:" + a8.Count());

var a9 = "   Hello world   ";
print("to upper:" + a9.Upper());
print("to lower:" + a9.Lower());
print("count:" + a9.Count("l"));
print("length:" + a9.Length());
print("replace:" + a9.Replace("world", "not"));
print("trim:" + a9.Trim());

```

# The Not Programming Language

## Build Instructions
On Unix, Linux, BSD, macOS, and Cygwin:
```
sudo apt-get update
sudo apt-get install libffi-dev libgmp-dev

make
make test
```

## Simple Code
```not
using param2: param from "~/lib/types/primary.not";

// import c/.. library shared
using println from "~/lib/c/shared.json";

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
	fun * (p1:int)
	{
		return b * p1;
	}
}

var fun1 = fun (p1:int) {
	return p1 * 1000;
};

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

```

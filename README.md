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
set "NOT_LIBRARY_PATH" in file "etc/environment" to path of lib folder,
windows: set "NOT_LIBRARY_PATH" in "Environment Variables" to path of lib folder

```csharp
using Print from "~/os";
Print("Hello World");
```

```csharp
using Print from "~/os";

var print_diamond = fun (n:int) {
	for (var i = 1;i < 2 * n; i += 1)
	{
		var spaces = (n - i) > 0 ? n - i : i - n;
		var stars = 2 * (n - spaces) - 1;
		Print(" " * spaces + "*" * stars);
	}
};

print_diamond(6);
```

```csharp
using Print from "~/os";

readonly var print_vowels = fun(str:string) {
	var vowels = {h:0, e:0, l:0, o:0, r:0};
	for (var c in str)
	{
		if (vowels.Contain(string(c)))
		{
			vowels[string(c)] += 1;
		}
	}

	Print ("Count of vowels in given string: " + vowels);
};

print_vowels("hello world");
```

```csharp
using Print from "~/os";

readonly var print_pattern = fun(str: string) {
	for (var i = 1; i < str.Length(); i += 1)
	{
		Print(str[0, i - 1]);
	}

	for (var i = str.Length(); i > 0; i -= 1) {
		Print(str[0, i - 1]);
	}
};
print_pattern("Hello World");
```

```csharp
using Print from "~/os";

readonly var print_alphabetical = fun(n:int) {
	var num = 0;
	for (var i = 1; i < n + 1; i += 1)
	{
		for (var j = 1; j < i + 1; j += 1)
		{
			Print('A' + num, end = " ");
			num += 1;
		}
		Print("");
	}
};

print_alphabetical(6);
```

how to use *arg?
```csharp
using Print from "~/os";

readonly var add = fun(*args:int) {
	var result = 0;
	for (var num in args)
	{
		result += num;
	}
	return result;
};

Print("sum :" + add(1, 2, 3, 4));
```

how to use **kwarg?
```csharp
using Print from "~/os";

readonly var greet = fun (**kwargs:string) {
	if (kwargs.Contain("name"))
	{
		Print("Hello, " + kwargs["name"]);
	}
	else
	{
		Print("Hello, stranger!");
	}
};

greet(name = "Alice");
greet(type = "string");
```

lambda functions?
```csharp
using Print from "~/os";

readonly var square = fun (x) -> x ** 2;
Print("square 5: " + square(5));
```

```csharp
using Print, Input from "~/os";

var data = Input("Enter your name?");
if (data)
{
	Print("Hello, " + data);
}
```

```csharp
using Print, File from "~/os";

// convert string to float
var b1 = float ("12e-1");

// convert string to int
var b2 = int ("12345");

// round
var b3 = float (1.3451, 2);

Print("hello world" + " " + b1 + " " + b2 + " " + b3 + " ");

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
		Print("f = " + p1);
		return fun2(p1 - 1);
	}
	Print("f2 = " + p1);
	return p1 * 1000;
};

var f1 = fun1(100);
Print("f1 = " + f1);

// new class
var f2 = A<int>(100000);
var f3 = f2 * 2;
Print(f2);

// typeof|instanceof
var f4 = typeof f2;
var f5 = f2 instanceof A<int>;

// access to item
var f6 = f4[2];

// slice -> [start, stop, step]
var f7 = a5[1, 2]; 
var f8 = a5[1, -1, 1];
Print(f8);

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
	Print(ex);
}

// forin loop
for loop1 (var key, value in ["hello", "world", 123])
{
	Print(value);
}

// object methods
var a7 = {a:1};

a7.Remove(key = "a");
a7.Add("b", 123);

Print(a7);

// tuple/sequence methods
var a8 = [0];

a8.Set(1, 10);
a8.Append(15);
a8.Insert(4, 15);
a8.Remove(2);

Print(a8);
Print("count tuple:" + a8.Count());

var a9 = "   Hello world   ";
Print("to upper:" + a9.Upper());
Print("to lower:" + a9.Lower());
Print("count:" + a9.Count("l"));
Print("length:" + a9.Length());
Print("replace:" + a9.Replace("world", "not"));
Print("trim:" + a9.Trim());


var file = File("text.txt", File.O_CREAT | File.O_RDWR);

if (file)
{
	Print("read file:" + file.Read());
	Print("write to file:" + file.Write(a9));
	Print("size of file:" + file.Seek(0, File.SEEK_END));
	Print("close file:" + file.Close());
}


```

using param2: param from "~/lib/types/primary.q";
using add, println from "~/lib/c/shared.json";

var f1 = add(5, 2);
println("hello world %d %d", f1, 33);

class B
{
	export static b = 1000000;
	export c = 0;
	export fun constructor(p1:int)
	{
		c = p1;
	}
}

class C extends (b1:B)
{
	export fun constructor(p1:int)
	{

	}
}

class A<T> extends (b1:B)
{
	export static a = 1000000;
	export b:T = 10;
	export fun constructor(p1:int)
	{
		b1.constructor(1000000);
		b = b * p1;
	}

	export fun Get() {
		return b;
	}

	export fun [](val:int, step:int = 1)
	{
		return b/val;
	}

	fun * (p1:int)
	{
		return b * p1;
	}
}

var fun1 = fun (p1:int) {
	return p1 * 1000;
};

var f2 = A<int>(100000);
var f3 = f2 * 1;
var f4 = string(typeof f2);
var f5 = f4[2];
var f6 = "hello world" + f2.b;
var f6 = "hello world" + f2.b;
export var sum = 0;
try {
	for loop1 (var a = 0;a < 1000000;a += 1)
	{
		sum += a;

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

for loop1 (var key, value in ["hello", "world", 123])
{
	println(string(value));
}
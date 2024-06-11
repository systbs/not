//using param2: param from "~/lib/types/primary.q";

class B
{
	export static b = 1000000;
	export c = 0;
	export fun Constructor(p1:int)
	{
		c = p1;
	}
}

class C extends (b1:B)
{
	export fun Constructor(p1:int)
	{

	}
}

class A<T> extends (b1:B)
{
	export static a = 1000000;
	export b:T = 10;
	export fun Constructor(p1:int)
	{
		b1.Constructor(1000000);
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
var f4 = string(f2);
var f5 = f4[2];
var f6 = "hello world" + f2.b;

export var sum = 0;
try {
	for loop1 (var a = 0;a < 1000000;a += 1)
	{
		sum += a;

		if (a > 500000)
		{
			throw f6;
			continue loop1;
		}
	}
}
catch(ex)
{
	string(ex);
}


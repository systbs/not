//using param2: param from "~/lib/types/primary.q";

class B
{
	export static b = 1000000;
	export c = 0;
	export fun Constructor(p1:int32)
	{
		c = p1;
	}
}

class C extends (b1:B)
{
	export fun Constructor(p1:int32)
	{

	}
}

class A<T extends B> extends (b1:B)
{
	export static a = 1000000;
	export b = 1000;
	export fun Constructor(p1:int32)
	{
		b1.Constructor(1000000);
		b = b * p1;
	}

	export fun Get() {
		return b;
	}

	export fun [](val:int32, step:int32 = 1)
	{
		return b/val;
	}

	fun * (p1:bigint)
	{
		return b * p1;
	}
}

var fun1 = fun (p1:int32) {
	return p1 * 1000;
};

var f2 = A<C>(1000);
var f3 = f2 * 1;

export var sum = 0;
for loop1 (var a = 0;a < f3;a += 1)
{
	sum += a;

	if (a > 500000)
	{
		continue loop1;
	}
}


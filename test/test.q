using param2: param from "~/lib/types/primary.q";

export readonly var fun1 = fun <G, T, R>(){

};

class B {
	export static b = 1000000;
}

class A extends (b1:B) {
	export static a = 1000000;
	export fun Constructor(p1:int32, **p2:int64, p3:int32)
	{

	}
}

var f1 = fun1<int32, int64, R = int32>;
var f2 = A(1, T=2, R=3, 4);

export var sum = 0;
for loop1 (var a = 0;a < 1000000;a += 1)
{
	sum += a;
	if (a > 500000)
	{
		continue loop1;
	}
}


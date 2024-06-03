using param2: param from "~/lib/types/primary.q";

export readonly var fun1 = fun <G, T, R>(){

};

class B {
	export static b = 500000;
}

class A extends (b1:B) {
	export static a = 1000000;
	fun Constructor()
	{

	}
}

var f1 = fun1<int32, int64, R = int32>;
var f2 = A.a;

export var sum = 0;
for loop1 (var a = 0;a < A.b;a += 1)
{
	sum += a;
	if (a > 500000)
	{
		continue loop1;
	}
}
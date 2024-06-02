using param2: param * 2 from "~/lib/types/primary.q";

export readonly var fun1 = fun <G>() {

};

export var sum = 0;
for loop1 (var a = 0;a < param2;a += 1)
{
	sum += a;
	if (a > 500000)
	{
		continue loop1;
	}
}
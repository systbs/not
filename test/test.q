using primary:* from "~/lib/types/primary.q";

export static class System
{
	static prop1 = {a : 2};

	export static fun Constructor(*args:string[]) : int64
	{
		var pow = args.where(fun(x) -> int64(x) < 10).select(fun(x) -> int64(x) ** 2);
	}
}

var a:int64 = 50 * 60;
for b (a = 1;a < 10;a += 1)
{
	print("value a=${a}");
}
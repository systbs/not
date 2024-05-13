
using * from "~/lib/types/primary.q";
using Array<T>: Array<T>, Dictionary<T, V>: Dictionary<T, V>, Set<V>:Set<V> from "~/lib/types/container.q";

export class A
{
	prop1:Any;

	export fun Constructor()
	{
	}

	export fun a2 (a1:int32) 
    {
    }
}

export class B extends (a:A) 
{
	export fun Constructor()
	{
	}

	export fun + (val : int32) : int64 
    {
    }

	export fun a2 (a1:float32) 
    {
    }
}

export class System
{
	prop1 = {a : 2};

	@Main
	export static fun Constructor(*args:string) : int64
	{
		var pow = args.where(fun(x) -> x < 10).select(fun(x) -> x ** 2);
	}
}

using * from "~/lib/types/primary.q";
using Array<T>: Array<T>, Dictionary<T, V>: Dictionary<T, V>, Set<V>:Set<V> from "~/lib/types/container.q";

export class A {
	prop1:Any;
	export fun Constructor()
	{
	}
	export fun a () 
    {

    }
}

export class B extends (a:A) {
	export fun Constructor()
	{
	}

	export fun + (val : Any) : Any 
    {

    }

	export override fun a2 () 
    {

    }
}

export class System<R>
{
	prop1 = {a:2};

	@Main
	export static fun Constructor(*args:Array<String>) : Int64
	{

	}
}
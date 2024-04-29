
using * from "~/lib/types/primary.q";
using Array<T>: Array<T>, Dictionary<T, V>: Dictionary<T, V>, Set<V>:Set<V> from "~/lib/types/container.q";

export class A {
	export fun Constructor() : A
	{
	}

	export fun + (val : Any) : Any 
    {

    }
}

export class System<R>
{
	prop1 = {a:2};

	@Main
	export static fun Constructor(*args:Array<String>) : Int64
	{
		try {

		}
		catch (e : fun<T>(a:Int32, b:Int64) -> Any)
		{
			var a:Any;

			var a1 = {b : 1, c : 2};       	// an object
			var a2 = {"b" : 1, "c" : 2};   	// a dictionary
			var a3 = {1, 2, 3};				// a set


			var {b, c} = a1;

			a = fun (a:Int32) : Int32 {
				throw (e = Any());
			}.b;
		}
	}
}
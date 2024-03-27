import {Short, Int, Long, Double, Float, String} : "~/types/statics";
import {Array<T>:Container.Array<T>} : "~/types/container";

class Int4
{
  export fun Constructor(a:Int, b:Int)
  {
  }

  export class Long {

  }
}

class Int3 extends (i4:Int4)
{
  export fun Constructor(a:Int, b:Int)
  {
  }
}

class System<R extends Int, T = Float>
{
  export fun Constructor(a:Int3.Long, b:Int)
  {
    print("Hello World!");
  }

  export fun R2(a:Int, b:String) : Int
  {
    print("Hello World!");
    var c = 1;

    try throw1<T> {
      throw1<T = Expection>(/*expection1*/);
    } 
    catch (e : T)
    {

    }
  }
}


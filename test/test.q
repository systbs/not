import {Short, Int, Long, Double, Float, String} : "~/types/statics";
import {Array<T>:Container.Array<T>} : "~/types/container";

class System<R extends Int, T = Float>
{
  export fun Constructor(a:Int, b:Int)
  {
    print("Hello World!");
  }

  export fun Constructor(a:Int, b:String)
  {
    print("Hello World!");
  }
}


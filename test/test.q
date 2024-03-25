import {Short, Int, Long, Double, Float, String} : "~/types/statics";
import {Array<T>:Container.Array<T>} : "~/types/container";

class System<R extends Int, T = Float>
{
  export fun Constructor()
  {
    print("Hello World!");
  }
}

class System<T extends Int>
{
  export fun Constructor()
  {

  }

  fun Action(a:Array<T>) : Int {
    var func1 = fun <T>(p1:Int) {};
  }
}
import {Short, Int, Long, Double, Float, String} : "~/types/statics";
import {Array<T>:Container.Array<T>} : "~/types/container";

class System<R extends Int, T = Float>
{
  export func Constructor()
  {
    print("Hello World!");
  }
}

class System<T extends Int>
{
  export func Constructor()
  {

  }

  func Action(a:Array<T>) : Int {
    
  }
}
import {Short, Int, Long, Double, Float, String} : "~/types/statics";
import {Array<T>:Container.Array<T>} : "~/types/container";

class System<R extends Int, T = Float>
{
  export fun Constructor(a:Int, b:Int)
  {
    print("Hello World!");
  }

  export fun R(a:Int, b:String) : Int
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


import {
  Short:Short, 
  Int:Int, 
  Long:Long, 
  Double:Double, 
  Float:Float, 
  String:String
  } : "~/types/statics";

import {
  Array<T>:Container.Array<T>
  } : "~/types/container";

export static class Int4
{
  export fun Constructor(a:Int, b:Int)
  {
  }

  export static class Long {

  }
}

export static class Int3 extends (i4:Int4)
{
  export fun Constructor(a:Int, b:Int)
  {

  }
}

export static class System<R extends Int, T = Float>
{
  export fun Constructor(a:Int3.Long, b:Int)
  {
    System.Print("Hello World!");
  }

  export fun R2(a:Int, b:String) : Int
  {
    System.Print("Hello World!");
    var c = 1;

    try throw1<T extends String> {
      throw1<T = String>(Expection(), "error message");
    } 
    catch (e : Expection, t:T)
    {

    }
    catch (e : Expection, t:Int)
    {

    }
  }
}
import {
  Short:Short, 
  Int:Int, 
  Long:Long, 
  Double:Double, 
  Float:Float, 
  String:String,
  Null:Null
  } : "~/types/statics";

import {
  Array<T>:Container.Array<T>,
  Dictionary<T, V>:Container.Dictionary<T, V>
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
  export Key:Int;

  export fun Constructor(a:Int3.Long, b:Int)
  {
    System.Print("Hello World!");
  }

  fun KeyEffect(value: Int) : Int
  {
    System.Print("Hello World!");
  }
}
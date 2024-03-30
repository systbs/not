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

  export protect static class Long {

  }
}

export static class Int3 extends (i4:Int4)
{
  export fun Constructor(a:Int, b:Int)
  {

  }
}

export class System<R extends Int, T = Float2> extends(h1:Int3)
{
  export Key2:Int4 @(set=KeyEffect1, get=KeyEffect2) = 1;

  export static enum Enum1 {
    key1=1,
    key2=2
  }

  export fun Constructor(a:Int3.Long, reference b:h1.Long)
  {
    System.Print("Hello World!");
  }

  fun KeyEffect(value: Int) : Int
  {
    System.Print("Hello World!");
  }
}
import {
  Char:Char, 
  Int16:Int16, 
  Int32:Int32, 
  Int64:Int64, 
  Float32:Float32, 
  Float64:Float64,
  Null:Null
  } : "~/types/primary.q";

import {
  Array<T>:Array<T>,
  Dictionary<T, V>:Dictionary<T, V>
  } : "~/types/container.q";


export class System<R extends Int32, T = Float32> extends (h1:Int32)
{
  export Key2:Int32 @(set=KeyEffect1, get=KeyEffect2) = 1;

  export fun Constructor(a:[Int32,Int32])
  {
    System.Print("Hello World!");
    var tuple1 = Tuple<Int32, Array<Int32>>();;
  }

  fun KeyEffect(value: Int32) : Int
  {
    System.Print("Hello World!");
  }
  
}
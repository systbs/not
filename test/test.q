import ("~/types/statics") : {
  Short, Int, Long, Double, Float, String
}

import ("~/types/container") : {
  Array<T>:Array<T>
}

class System<R extends Int, T = Float>
{
  export func Constructor()
  {

  }

  func action(a:Array<T>) : Int {
    
  }
}

class System<T extends Int>
{
  export func Constructor()
  {

  }

  func action(a:Array<T>) : Int {
    
  }
}
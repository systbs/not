export class Int<T>
{
  func constructor (value:String) -> Int
  {
   
  }

  func >> (value:Int) -> Int
  {
    
  }
}

export class String extends (v : Int)
{
  func constructor (value:String) -> String
  {
    
  }
}

func constructor(args:Int) -> Int
{
  return teacher<Int<Int>>(10);
}


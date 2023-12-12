export class (T extends string & int) task1
{
  export class (R extends string & int) task2
  {
    
  }
}


export class (T extends string, R) A
{
  a:int;
  func init {
    a = task1{T}.task2{R};

    var b:int = 1;
    if init (1){

    }
  }
}

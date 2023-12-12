export class (T extends string & int) task1
{
  export class (T extends string & int) task2
  {
    
  }
}


export class (T extends string, R) A
{
  func init {
    a = task1{T}.task2{R};
  }
}

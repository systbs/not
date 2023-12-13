export class (T extends int) task
{
  export a:int;
  export static b:int = 1;

  func init {

  }
}

export func init {
  var t:task{int} = task();
}
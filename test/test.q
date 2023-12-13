export class (T extends int) task
{
  export a:int;
  export static b:int = 1;

  func init {

  }

  export class (R extends int) task1
  {

  }
}

export func (a:task.task1) init {
  var t:task{int} = task();
}
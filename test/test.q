export class (T) task
{
  export class (R) task1
  {

  }
}

export func (R, T, a:task{R}.task1{T}) init {
  var t:task{int} = task{int}();
}
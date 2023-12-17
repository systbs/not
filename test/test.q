export class (W, R) task 
{

}

export class (T) task
{
  func init(T, T) {

  }

  export class (R) task1
  {

  }
}

export func (R, T, a:task{R}.task1{T}) init {
  readonly var t:task{T} = task{T}(1, 2);
}


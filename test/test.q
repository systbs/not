export class (W, R) task 
{

}

export class (T) task
{
  export class (R) task1
  {

  }
}

export func (R, T, a:task{R}.task1{T}, b:T[]) init {
  readonly var t:task{T} = task{int}();
}
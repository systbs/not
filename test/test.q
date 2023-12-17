
type int: typeof 0;
type char: typeof ' ';
type nil: typeof null;
type string: typeof "simple text";
type boolean: typeof true;

export class (W, R) task 
{

}

export class (T) task
{
  func init(t1:int, t2:int) {

  }

  export class (R) task1
  {

  }
}

export func (R, T, a:task{R}.task1{T}) init {
  readonly var t:task{T} = task{T}(1, 2);
}


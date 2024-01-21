import (T) task2 ("~/libs/task", promise, task{T, R}:task{T}.find{R});
import (T) task3 ("~/libs/task", promise, t{T}:task{int}.find{T});

export class (W=int, R extends string) teacher extends (p:person)
{
  static readonly version:int = 0;
  register_date:date;
  export func init (name:string, degree:string) 
  {

  }

  export func init
  {
    register_date = date();
  }

  export protected func get_date {
    return register_date;
  }

  export func + (readonly p:person) {

  }
}

func init(args:string[])
{
  var t1 = teacher();
  var t2 = teacher{R=string}("peyman", "bachelor");
}

type (T) A extends (b:B{T}, c:C{T}):
{

}

func (a:A{T}, T) fnA {
  
}



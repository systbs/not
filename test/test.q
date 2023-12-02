import ("~/lib/", ar:array.array, map)

export class (T = string, R) A extends (a: ar{T}.ar, m: map{T,R})
{
  export b:int;

  func init 
  {
    a();
    m();
    b = a.b + m.b;
  }

  export func get (a1:int, a2:int)
  {

  }

  export func get (a1:int, a2:string)
  {

  }

  class A {

  }
}

export type (T) G extends (t:T) :
{

}

export type (T) Y extends (t:T) :
{

}

func (g:G{T}, y:Y{T}, T extends T = T) get 
{

}

export type (T) U extends (g:G{T}, y:Y{T}) :
{
  a1:string,
  a2:int
}

import ("~/lib/", ar:array.array, map)

class (T = string, R) ar extends (a: ar{T}.ar, m: map{T,R})
{
  export b:int;

  func init 
  {
    a();
    m();
    b = a.b + m.b;
  }

  export func set (a2:ar{T})
  {

  }

  export static func (S) get ()
  {

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

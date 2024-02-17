export class string {

}

enum result {
  a = 1,
  b
}

export class int {
  

  func init (arg:string)
  {

  }

  init:int;
}

func init(args:int)
{
  var t1 = teacher();
  var t2 = teacher{R=string}("peyman", "bachelor");
}


export class (R extends int) person 
{
  export class (R extends int = int) person 
  {

  }
}



export class (W=int, R extends string) teacher extends (p:person)
{
  
}


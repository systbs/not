export class string {

}

export class int {
  init:int;

  func init (arg:string)
  {

  }
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


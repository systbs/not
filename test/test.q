
export class string {

}

export class int {
  func init {

  }
  func init {
    
  }
}



export class (R extends string) person 
{
  export class (R extends int) person 
  {

  }
}



export class (W=int, R extends string) teacher extends (p:person)
{
  
}

func init(args:array{string})
{
  var t1 = teacher();
  var t2 = teacher{R=string}("peyman", "bachelor");
}

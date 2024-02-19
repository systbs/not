export class string {

}

export class (T) int {

}

import ("~/lib/types.q",
  int{T, K} : property{T}.field{K}
);

var int = 1;
var a = 2;

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


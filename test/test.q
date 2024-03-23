import ("~/types/statics", Short, Int, Long, Double, Float, String);
import ("~/types/container", Array<T>);

class Int3 {
  export func constructor()
  {

  }
  export protect class Long {
    export func constructor()
    {

    }
  }
}

class Int2 extends (int3:Int3) {
  export func constructor()
  {

  }
}

class Int extends (int2:Int2) {
  export func constructor()
  {

  }
}

class System
{
  export func constructor()
  {

  }

  func action (a:Int) : Int {
    var {norm3:norm} = 1.1;
  }
}
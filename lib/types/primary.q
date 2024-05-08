@General
@Allocable
export class Any 
{
    export fun Constructor() : Any
    {

    }
}

@Null
export class Null 
{
    export fun Constructor() : Null
    {

    }
}

@Void
export class Void
{
    export fun Constructor() : Void
    {

    }
}

export class UInt8
{
    @Malloc(1)
    value: Any;

    export fun Constructor(value: Any) : UInt8
    {

    }
}

export class UInt16
{
    @Malloc(2)
    value: Any;

    export fun Constructor(value: Any) : UInt16
    {

    }
}

export class UInt32
{
    @Malloc(4)
    value: Any;
 
    export fun Constructor(value: Any) : UInt32
    {

    }
}


export class UInt64 
{
    @Malloc(8)
    value: Any;

    export fun Constructor(value: Any) : UInt64
    {

    }
}


export class Int8
{
    @Malloc(1)
    value: Any;

    export fun Constructor(value: Any) : Int8
    {

    }
}

export class Int16
{
    @Malloc(2)
    value: Any;

    export fun Constructor(value: Any) : Int16
    {

    }
}

export class Int32
{
    @Malloc(4)
    value: Any;

    export fun Constructor(value: Any) : Int32
    {

    }
}

export class Int64 
{
    @Malloc(8)
    value: Any;

    export fun Constructor(value: Any) : Int64
    {

    }
}

export class Float32
{
    @Malloc(4)
    value: Any;

    export fun Constructor(value: Any) : Float32
    {

    }
}

export class Float64
{
    @Malloc(8)
    value: Any;

    export fun Constructor(value: Any) : Float64
    {

    }
}

export class Complex32
{
    real: Float32;
    image: Float32;

    export fun Constructor(value: Any) : Complex32
    {
        
    }
}

export class Complex64 
{
    real: Float64;
    image: Float64;

    export fun Constructor(value: Any) : Complex64
    {

    }
}

export class Char 
{
    @Malloc(size=1)
    value: Any;

    export fun Constructor(value: Any) : Char
    {

    }
}
@General
@Mallocable
export class Any 
{
    export fun Constructor()
    {

    }
}

@General
@Null
export class Null 
{
    export fun Constructor()
    {

    }
}

export class UInt8
{
    @Malloc(1)
    value: Any;

    export fun Constructor(value: Any)
    {

    }
}

@Union(UInt8)
export class UInt16
{
    @Malloc(2)
    value: Any;

    export fun Constructor(value: Any)
    {

    }
}

@Union(UInt8, UInt16)
export class UInt32
{
    @Malloc(4)
    value: Any;
 
    export fun Constructor(value: Any)
    {

    }
}


@Union(UInt8, UInt16, UInt32)
export class UInt64 
{
    @Malloc(8)
    value: Any;

    export fun Constructor(value: Any)
    {

    }
}


export class Int8
{
    @Malloc(1)
    value: Any;

    export fun Constructor(value: Any)
    {

    }
}

@Union(Int8)
export class Int16
{
    @Malloc(2)
    value: Any;

    export fun Constructor(value: Any)
    {

    }
}

@Union(Int8, Int16)
export class Int32
{
    @Malloc(4)
    value: Any;

    export fun Constructor(value: Any)
    {

    }
}

@Union(Int8, Int16, Int32)
export class Int64 
{
    @Malloc(8)
    value: Any;

    export fun Constructor(value: Any)
    {

    }
}

export class Float32
{
    @Malloc(4)
    value: Any;

    export fun Constructor(value: Any)
    {

    }

    export fun -() : Float32
    {

    }
}

@Union(Float32)
export class Float64
{
    @Malloc(8)
    value: Any;

    export fun Constructor(value: Any)
    {

    }
}

export class Complex32
{
    base: Float32;
    image: Float32;

    export fun Constructor(value: Any)
    {
        
    }
}

@Union(Complex32)
export class Complex64 
{
    base: Float64;
    image: Float64;

    export fun Constructor(value: Any)
    {

    }
}

export class Char 
{
    @Malloc(size=1)
    value: Any;

    export fun Constructor(value: Any)
    {

    }
}
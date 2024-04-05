@General
export class Any 
{
    export fun Constructor()
    {

    }
}

@General
export class Null 
{
    export fun Constructor()
    {

    }
}

export class UInt8
{
    value: 1;

    export fun Constructor(value: Any)
    {

    }
}

@Union(UInt8)
export class UInt16
{
    value: 2;

    export fun Constructor(value: Any)
    {

    }
}

@Union(UInt8, UInt16)
export class UInt32
{
    value: 4;
 
    export fun Constructor(value: Any)
    {

    }
}


@Union(UInt8, UInt16, UInt32)
export class UInt64 
{
    value: 8;

    export fun Constructor(value: Any)
    {

    }
}


export class Int8
{
    value: 1;

    export fun Constructor(value: Any)
    {

    }
}

@Union(Int8)
export class Int16
{
    value: 2;

    export fun Constructor(value: Any)
    {

    }
}

@Union(Int8, Int16)
export class Int32
{
    value: 4;

    export fun Constructor(value: Any)
    {

    }
}

@Union(Int8, Int16, Int32)
export class Int64 
{
    value: 8;

    export fun Constructor(value: Any)
    {

    }
}

export class Float32
{
    value: 4;

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
    value: 8;

    export fun Constructor(value: Any)
    {

    }
}

export class Complex32
{
    base: 4;
    image: 4;

    export fun Constructor(value: Any)
    {
        
    }
}

@Union(Complex32)
export class Complex64 
{
    base: 8;
    image: 8;

    export fun Constructor(value: Any)
    {

    }
}

export class Char 
{
    value: 1;

    export fun Constructor(value: Any)
    {

    }
}
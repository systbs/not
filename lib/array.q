export type array{V} : V[];
export func (a:array{V}) length{V}() -> int :
{
  var i:int = 0;
  for(a[i])
  {
    i += 1;
  }
  return i;
}

export func (a:array{V}) reduce{R, V}(f:func(result:R, current:V, index:int, arr:array{V}) -> R, initial:R) -> R :
{
  var result:R = initial;
  for(i := 0;i < a.length();i += 1)
  {
    result = f(result, a[i], i, a);
  }
  return result;
}

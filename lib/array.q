export type (V) array: V[];
export func (a:array{V}, V) length {
  var i:int = 0;
  for(a[i])
  {
    i += 1;
  }
  return i;
}

export func (a:array{V}, R, V) reduce(f:fn, initial:R) {
  var result:R = initial;
  for(i := 0;i < a.length();i += 1)
  {
    result = f(result, a[i], i, a);
  }
  return result;
}

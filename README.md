# qalam
interpreter

# syntax now
## variable
```js
[var name] = [value];
schame: {
  a = "sample text";
}
```

## class(schame)
```js
[schame name]: {
  [expr...]
}
```

## inheritance
```js
[schame name]:([schame_extend_name1],[schame_extend_name2], {
  [expr...]
}
```

## sample code
```js
schame1: {
  a = "sample text 1";
}

schame2: {
  b = "sample text 2";
}

schame3: (schame1, schame2, {
  c = "sample text 3";
  print("hello world");
});

schame = eval(schame3);
print(schame.a);
```
#todo list
1 - schame for variable
```js
i: schameInt;
i = 1;
```
2 - schame parameter
```js
fn: (a,b,c) :> {
  return a + b + c;
}
a = eval(fn, 1,2,3);
// or
a = fn = (1,2,3);
```
3 - math lib for operator
4 - ...


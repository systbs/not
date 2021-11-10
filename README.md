# qalam
interpreter

# syntax now
## variable
```js
[var name] = [value];
schame: def {
  a = "sample text";
}
```

## class(schame)
```js
[schame name]: def {
  [expr...]
}
```

## inheritance
```js
[schame name]:([schame_extend_name1],[schame_extend_name2], def {
  [expr...]
});
```

## sample code
```js
schame1: def {
  a = "sample text 1";
}

schame2: def {
  b = "sample text 2";
}

schame3: (schame1, schame2, def {
  c = "sample text 3";
  print("hello world");
  fn: def {
    (i, j, k) = params;
    return i+j+k;
  }
});

schame = eval(schame3);
print(schame.a);

sum = schema.fn(1,2,3);
// sum = schema.fn = (1,2,3);
print(sum);
```


# todo
1 - schame for variable
```js
i: schameInt;
i = 1;
// if use a number after def operator":", malloc memory to schema variable
j: 32;
// malloc 32 bit to j
```
2 - math lib for operator, memory management, threading, compile, ...

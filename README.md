# qalam
interpreter

# syntax && code
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
    [i, j, k] = params;
    return i+j+k;
  }
});

schame = schame3();
print(schame.a);

sum = schema.fn(1,2,3);
// sum = schema.fn = (1,2,3);
print(sum);
//output: 6

a = 1;
b = 2;
c = 3;
(b, c, a) = (a, b, c);
print(c);
//output: 3
```

# In the future
convert generated code to assembly code and then to machin code
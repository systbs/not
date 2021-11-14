# qalam
interpreter

# syntax && code
```js
class1: def {
  a = "sample text 1";
}

class2: def {
  b = "sample text 2";
}

class3: (class1, class2, def {
  c = "sample text 3";
  print("hello world");
  fn: def {
    [i, j, k] = params;
    return i+j+k;
  }
});

ca = class3();
print(ca.a);

sum = ca.fn(1,2,3);
// sum = ca.fn = (1,2,3);
print(sum);
//output: 6

a = 1;
b = 2;
c = 3;
(b, c, a) = (a, b, c);
print(c);
//output: 3

print("lorem text %n %s\n", 19, "amet direm");
//output: lorem text 19 amet direm
```

# In the future
## import
(class1,class2) = import "file_path_name.q";
## memory management in parallel mode
## ...
## compile
convert generated code to assembly code and then to machin code

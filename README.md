# qalam
interpreter

# syntax && code
```js
{class4,class5} = import "test3.q";

class1: def {
  a = "sample text 1";
}

class2: def {
  b = "sample text 2";
}

num2 = 2;
class3: (class1, class2, def {
  c = "sample text 3";
  num2 = 3;
  print("hello world, this.num2 %n, super.num2 %n\n", this.num2, super.num2);
  fn: def (i,j,k) {
    return i+j+k;
  }

  num = 2;
  +: def (val) {
    return super.num + val;
  }
});

ca = class3();
print(ca.a);

sum = ca.fn(1,2,3);
// sum = ca.fn = (1,2,3);
print(sum);
//output: 6

print(ca + 3);
//output: 5

a = 1;
b = 2;
c = 3;
{b,c,a} = {a, b, c};
print(c);
//output: 3

print("lorem text %n %s\n", 19, "amet direm %n", 13);
//output: lorem text 19 amet direm 13
```
# contribute
improve or find bugs or fix them

# In the future
## 1. Multi threading ...
## 2. System functions
## 3. Memory management
## 4. Artificial intelligence  
## ...
## 6. compile as assembly code
convert generated code to assembly code and then to machin code

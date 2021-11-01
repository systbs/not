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
});
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
# todo
1 - schame for variable
```js
i: schameInt;
i = 1;
// if use a number after def operator":", malloc memory to schema variable
j: 32;
// malloc 32 bit to j
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
3 - create a new phase after parser for remove phase dynamic schema allocate to static schema and increase speed. After making the changes, the inital variables are applied and the program enters the phase whithout the need to check the information. 
```js
[schame name]:([schame_extend_name1],[schame_extend_name2], {
  [expr...]
});

change to :

[schame name] extends [schame_extend_name1],[schame_extend_name2] {
  [expr...]
});

it's mean in eval phase:
ENT
jmp schame_extend_name1 // and return next line
jmp schame_extend_name2 // and return next line
// if have parameter
IMM param1 LONG
PUSH
SD
IMM param2 LONG
PUSH
SD
  ...
RET
LEV
// then run eval phase
```
4 - math lib for operator, memory management, threading, ...


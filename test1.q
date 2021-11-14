(category4, 2) = import "test2.q";

p = category4();
print("%s\n", p.w);

category1: def {
	a = "simple text 1";
};

category2: def {
	b = "simple text 2";
};

category3: (category1, category2, def {
	c = "simple text 3\n";
	print("Hello World\n");
	fn: def {
		[i, j, k] = params;
		print(params[1]);
		return i + j + k;
	}
});

d = category3();
f = d.fn(1,2,3);

print(d.b);
print(f);

print("\n");
g = 2 * 3 + 3 * 2 * 4;
print(g);

i = 1;
while(i <= 5){
	j = 1;
	while(j <= i) {
		j = j + 1;
		if(j > 3){
			print(j);
		}else{
			continue;
		}
	}
	i = i + 1;
	print("\n");
}


a = 1;
b = 2;
c = 3;

(b,c,a) = (a, b, c);

print("%n \n", b);
print("lorem text %n %s\n", 19, "amet direm");
category1: {
	a = "a simple text 1";
};

category2: {
	b = "a simple text 2";
};

category3: (category1, category2, {
	c = "a simple text 3";
	print("hello world");
});

d = eval(category3);

print(d.c);
print(d.a);
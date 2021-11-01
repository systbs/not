category1: {
	a = "a simple text 1";
};

category2: {
	b = "a simple text 2";
};

category3: (category1, category2, {
	c = "a simple text 3\n";
	print("Hello World\n");
});

d = category3 = null;

print(d.c);

i = 1;
while(i < 100000){
	i = i + 1;
}

print(i);
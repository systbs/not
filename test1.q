category1: {
	a = "simple text 1";
};

category2: {
	b = "simple text 2";
};

category3: (category1, category2, {
	c = "simple text 3\n";
	print("Hello World\n");
});

d = category3 = null;

print(d.c);

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
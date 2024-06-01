
var sum = 0;
for loop1 (var a = 0;a < 1000000;a += 1)
{
	sum += a;
	if (a > 500000)
	{
		continue loop1;
	}
}
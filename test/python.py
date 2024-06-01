sum = 0
i = 0
while i < 10_000_000:
    sum += i
    i += 1
    if i > 500_000:
        i += 5
        continue

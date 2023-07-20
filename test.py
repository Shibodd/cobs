def get_values(file):
  with open(file, 'r') as f:
    ans = {}
    for line in f:
      try:
        val = line.split(' ')[1].strip()
      except:
        continue
      if val not in ans:
        ans[val] = 1
      else:
        ans[val] = ans[val] + 1
  return ans
  
values = get_values('out.txt')
print([(k,v) for k,v in values.items() if v <= 1])
def func_for_line(input_line):
    counter = input_line[:input_line.find(';')] if ';' in input_line else input_line
    command = input_line[input_line.find(';')+1:] if ';' in input_line else input_line
    return counter, command
z1, z2 = func_for_line(input_line = '3;saasda shi13')
print(z1, z2)
#print(f"counter:{counter} and command:{command}")
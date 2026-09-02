from data_processing import string_to_float_array, string_to_string_array, string_to_float_array_without_last_symbol

line = "1 3 56 7 l"
string_array = string_to_string_array(line)
float_array = string_to_float_array_without_last_symbol(line)
print(string_array)
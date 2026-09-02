import math
import numpy as np

def delete_last_symbol_in_string_array(string_array):
    string_array.pop()
    return string_array

def string_to_string_array(string_data):
    data_array = string_data.split()
    return data_array

def string_array_to_float_array(string_array) -> list[float]:
    float_array = []
    for element in string_array:
        if element == "nan":
            float_array.append(math.nan)
        else:
            float_array.append(float(element))
    return float_array

def string_to_float_array(string_data):
    return string_array_to_float_array(string_to_string_array(string_data))

def string_to_float_array_without_last_symbol(string_data):
    return string_array_to_float_array(delete_last_symbol_in_string_array(string_to_string_array(string_data)))

def R_T_data_creation(input_2dim_data_array):
    input_data = input_2dim_data_array
    T_for_R1 = (math.sqrt(2)*input_data[:, 1] + math.sqrt(2)*input_data[:, 5] + input_data[:, 7])/(2*math.sqrt(2)+1)
    T_for_R2 = (math.sqrt(2)*input_data[:, 2] + input_data[:, 7] + input_data[:, 8])/(math.sqrt(2)+2)
    T_for_R3 = (math.sqrt(2)*input_data[:, 3] + input_data[:, 8] + input_data[:, 9])/(math.sqrt(2)+2)
    T_for_R4 = (math.sqrt(2)*input_data[:, 4] + math.sqrt(2)*input_data[:, 6] + input_data[:, 9])/(2*math.sqrt(2)+1)
    T_for_R5 = (input_data[:, 7] + math.sqrt(2)*input_data[:, 10] + input_data[:, 12])/(math.sqrt(2)+2)
    T_for_R6 = (input_data[:, 7] + input_data[:, 8] + input_data[:, 12] + input_data[:, 13])/4
    T_for_R7 = (input_data[:, 8] + input_data[:, 9] + input_data[:, 13] + input_data[:, 14])/4
    T_for_R8 = (input_data[:, 9] + math.sqrt(2)*input_data[:, 11] + input_data[:, 14])/(math.sqrt(2)+2)
    T_for_R9 = (input_data[:, 12] + math.sqrt(2)*input_data[:, 15] + input_data[:, 17])/(math.sqrt(2)+2)
    T_for_R10 = (input_data[:, 12] + input_data[:, 13] + input_data[:, 17] + input_data[:, 18])/4
    T_for_R11 = (input_data[:, 13] + input_data[:, 14] + input_data[:, 18] + input_data[:, 19])/4
    T_for_R12 = (input_data[:, 14] + math.sqrt(2)*input_data[:, 16] + input_data[:, 19])/(math.sqrt(2)+2)
    T_for_R13 = (input_data[:, 17] + math.sqrt(2)*input_data[:, 20] + math.sqrt(2)*input_data[:, 22])/(2*math.sqrt(2)+1)
    T_for_R14 = (input_data[:, 17] + input_data[:, 18] + math.sqrt(2)*input_data[:, 23])/(math.sqrt(2)+2)
    T_for_R15 = (input_data[:, 18] + input_data[:, 19] + math.sqrt(2)*input_data[:, 24])/(math.sqrt(2)+2)
    T_for_R16 = (input_data[:, 19] + math.sqrt(2)*input_data[:, 21] + math.sqrt(2)*input_data[:, 25])/(2*math.sqrt(2)+1)

    new_columns = [T_for_R1, T_for_R2, T_for_R3, T_for_R4, T_for_R5, T_for_R6, T_for_R7, T_for_R8,
                   T_for_R9, T_for_R10, T_for_R11, T_for_R12, T_for_R13, T_for_R14, T_for_R15, T_for_R16]
    output_2dim_data_array = input_2dim_data_array
    for column in new_columns:
        output_2dim_data_array = np.column_stack((output_2dim_data_array, column))
    return  output_2dim_data_array

def R_T_data_creation_test(input_2dim_data_array):
    input_data = input_2dim_data_array
    T_for_R1 = (math.sqrt(2)*input_data[:, 0] + math.sqrt(2)*input_data[:, 1] + input_data[:, 2])/(2*math.sqrt(2)+1)
    #output_2dim_data_array = input_2dim_data_array
    #output_2dim_data_array = np.append(output_2dim_data_array, [T_for_R1], axis=1)
    #output_2dim_data_array = np.column_stack((output_2dim_data_array, T_for_R1, T_for_R1))
    extra_cols = np.array([T_for_R1, T_for_R1, T_for_R1])
    for array in extra_cols:
        output_2dim_data_array = np.column_stack((input_2dim_data_array, array))
    #print(extra_cols)
    #output_2dim_data_array = np.concatenate((output_2dim_data_array, extra_cols), axis=1)
    return  output_2dim_data_array

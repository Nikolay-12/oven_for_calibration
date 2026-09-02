def is_valid_floating_point(new_value: str):
    if len(new_value) == 0:
        return True
    new_value = new_value.replace(",", ".")
    try:
        float(new_value)
    except:
        return False
    else:
        return True

def is_valid_non_zero_integer(new_value: str):
    if len(new_value) == 0:
        return True
    try:
        num = int(new_value)
        if num > 0:
            return True
    except:
        return False
    else:
        return True

def is_valid_stages(input: str) -> bool:
    if input == "":
        return True

    parts = input.split('_')

    try:
        # Проверяем первое число
        if len(parts) >= 1:
            if parts[0] == "":
                return False
            first_num = int(parts[0])
            if first_num <= 0:
                return False

            expected_parts = first_num + 1

            # Если частей больше чем нужно - запрещаем
            if len(parts) > expected_parts:
                return False

            # Проверяем остальные части
            for i, p in enumerate(parts[1:], start=1):
                if p == "":
                    # Пустая часть допустима только если это последняя часть
                    if i == len(parts) - 1:
                        continue
                    else:
                        return False

                # Проверяем, что это число (целое или с плавающей точкой)
                try:
                    float(p)
                except ValueError:
                    return False

            return True

    except ValueError:
        return False

    return False
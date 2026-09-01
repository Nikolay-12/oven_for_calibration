Программа для работы с набором термопар (без программ Command_Analyzer и Command_Parser). 
Предназначена для управления большим количеством термопар с использованием сдвигого регистра 74HC595. Для считывания температур используются термопары MAX6675.


Файл thermocouples:
Содержит несколько функций:
void init() - инициализация пинов (термопар и регистра) для работы с термопарами по интерфейсу SPI
void read_all_temp_once() - считывает показания всех термопар в переменные instances[i].temp_celsius один раз
void read_all_temp() - считывает показания всех термопар в переменные instances[i].temp_celsius до прерывания
void read_temp_from_one_thermocouple_once(uint8_t thermocouple_index) - считывает показания одной выбранной термопары в переменную instances[i].temp_celsius один раз
void read_temp_from_one_thermocouple(uint8_t thermocouple_index) - считывает показания одной выбранной термопары в переменную instances[i].temp_celsius до прерывания
void stop_reading_temp() - останавливает считывание показаний
double get_temp_celsius(uint8_t thermocouple_index) - посылает запрос на получение температуры с конкретной термопары
double get_average_temp_celsius() - посылает запрос на получение усредненной по всем термопарам температуры 
void average_temp_celsius() - усредняет показания со всех термопар
void send_temp_point(uint8_t thermocouple_index) - отправляет пользователю в удобном виде показания с выбранной термопары
void send_temp_array() - отправляет пользователю в удобном виде показания со всех термопар

Файл 74HC595_register_and_MAX6675_thermocouple:
Содержит два namespace register_74HC595 и thermocouple_MAX6675.
namespace register_74HC595 содержит следующие функции:
void setting_up_reg_pins() - инициализация пинов регистра
void set_0_on_DS_pin() - выставляет 0 на пине DS (пин данных)
void set_1_on_DS_pin() - выставляет 1 на пине DS (пин данных)
void shift_bits_in_reg() - сдвигает биты в регистре на одну позицию к концу
void save_data_in_reg() - сохраняет биты, т.е. передает их на выходы
void set_11111111_in_reg() - устанавливает 11111111 на выходах регистра
void add_0_to_reg() - добавляет 0 в начало регистра, т.н. "путешествующий" 0
void shift_and_save_data_in_reg() - сдвигает и сохраняет биты в регистре
void set_byte_in_reg(uint8_t value) - устанавливает определенный байт на выходе регистра

namespace thermocouple_MAX6675
{
  void setting_pin_values_for_SPI() - инициализация пинов термопар
  double readCelsius() - считывает показания с термопары
}
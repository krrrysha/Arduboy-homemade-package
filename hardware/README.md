
# Варианты сборки приставки

## 1. Вариант на основе платы проекта (в разработке)
[kit](./kit/)

В папке имеется схема версии v0.3b и STL-файл корпуса платы v0.1

![Принципиальная схема](./kit/v03b.png)

![Корпус (STL)](./kit/kit_v01_vaza.stl)

![Ручка джойстика (STL)](./kit/Direct.stl)



## 2. Вариант на основе Funduino Joystick Shield и плата в формате UNO (AVR или MIK32)
[joystick_shield_case](./joystick_shield_case/)

* Схема соединений 

![Схема соединений](./joystick_shield_case/JoystikShieldUno.png)

![Процесс сборки (видео)](./joystick_shield_case/video_manual.mp4)

![Подтяжка i2c к питанию (фото)](./joystick_shield_case/i2c_pull_up.jpg)

![Основание для 3d-печати (STL)](./joystick_shield_case/JS_btm1.stl)

![Шаблон отверстий для основания из оргстекла](./joystick_shield_case/template.pdf)

## 3. Устаревший вариант на основе платы стороннего клона Arduboy с маркетплейса. Используется с модулем в формате Nano (AVR или MIK32)

*(плата устанавливается на верхнюю часть корпуса)* 

* Детали корпуса. Файлы в формате STL для 3d-печати:
[2_inch_oled_case](./2_inch_oled_case/)

* Корпус (Общий вид) 

![Корпус (Общий вид)](./2_inch_oled_case/overview.jpg)

* Плата (приблизительные габаритные размеры)

![Плата (приблизительные габаритные размеры)](./2_inch_oled_case/approximate_dimensions_of_the_board.png)

----------------------------------


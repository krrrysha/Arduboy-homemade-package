#include <Arduboy2.h>
Arduboy2 arduboy;

#include "riscv_csr_encoding.h"
#include "csr.h"
#include "mcu32_memory_map.h"
#include "spifi.h"
  
uint32_t max_game_address=0x80120000;
const char unitName[6]={'E','L','B','E','A','R'};
typedef void (*game_func_t)(void);


// Структура одной игры
typedef struct {
    const char* name;       // Название игры
    uint32_t address;       // Адрес в памяти для запуска
    const uint8_t* icon;    // Указатель на иконку 16x16 px
} GameEntry;


const uint8_t icon_game1[] PROGMEM = {
  0xFF, 0xFF, // строка 0
  0xCF, 0xCF, // строка 1
  0xFF, 0xFF, // строка 2
  0xFF, 0xFF,
  0xCF, 0xCF,
  0xFF, 0xFF,
  0x00, 0x00,
  0xFE, 0x06,
  0x7E, 0x7E,
  0x06, 0xFE
};


const uint8_t icon_game2[] PROGMEM = {
  0xFF, 0xFF, // строка 0
  0xCF, 0xCF, // строка 1
  0xFF, 0xFF, // строка 2
  0xFF, 0xFF,
  0xCF, 0xCF,
  0xFF, 0xFF,
  0x00, 0x00,
  0xFE, 0x06,
  0x7E, 0x7E,
  0x06, 0xFE
};

const uint8_t icon_game3[] PROGMEM = {
  0xFF, 0xFF, // строка 0
  0xCF, 0xCF, // строка 1
  0xFF, 0xFF, // строка 2
  0xFF, 0xFF,
  0xCF, 0xCF,
  0xFF, 0xFF,
  0x00, 0x00,
  0xFE, 0x06,
  0x7E, 0x7E,
  0x06, 0xFE
};


// Пример списка игр (хранится в PROGMEM)
const GameEntry games[] PROGMEM = {
    {"doom-nano",    0x80040000, icon_game1}, // 28x4k (в реальности на ~16k меньше)
    {"Arduboy3d",    0x80060000, icon_game1}, // 9x4k
    {"MBaloons",     0x80070000, icon_game1}, // 9x4k
    {"StarHonor",    0x80080000, icon_game1}, // 3*64k
    {"Unicorn Dash", 0x800C0000, icon_game1}, //  5*4k
    {"Bomberman",    0x800E0000, icon_game1}, // 4*4k
    {"TankForce",    0x80100000, icon_game1}, // 12 * 4k
    {"Dark'n'Under", 0x80110000, icon_game1}, // 12 * 4k
};






const int gameCount = sizeof(games) / sizeof(GameEntry);
int selected = 0;


void goto_game(uint32_t address) {
    asm volatile (
        "mv   ra, %[addr]\n\t"  // Перемещаем переданный адрес в регистр ra
        "jalr ra"               // Переходим по адресу, используя jalr
        :
        : [addr] "r" (address)  // Используем регистр для передачи адреса
        : "ra"                  // Указываем, что регистр ra будет изменен
    );
}
// Функция отрисовки меню
// Константы
#define MENU_TOP 0                  // Начальная Y-координата первого пункта
#define MENU_ITEM_HEIGHT 18         // Высота строки меню
#define MENU_VISIBLE_ITEMS 4        // Количество видимых пунктов на экране
#define SCREEN_CENTER_Y (64 / 2)    // Центр экрана Arduboy (по Y)

void drawMenu(const GameEntry* games, int selected) {
    arduboy.clear();

    // Фиксируем положение стрелочки
    int arrowY = 0;
    arduboy.setCursor(18, arrowY);
    arduboy.print(">");

    // Определяем, сколько пунктов отображаем
    int firstItem = selected;
    if (firstItem < 0) firstItem = 0;

    int lastItem = firstItem + MENU_VISIBLE_ITEMS;
    if (lastItem > gameCount) lastItem = gameCount;

    // Отображаем пункты меню, центрируя выделенный пункт
    for (int i = firstItem; i < lastItem; i++) {
        int yPos = MENU_TOP + (i - firstItem) * MENU_ITEM_HEIGHT;

        // Позиция текущего пункта
        arduboy.setCursor(36, yPos); // Смещено от стрелочки
        arduboy.print(games[i].name);

        // Рисуем иконку слева от текста
        arduboy.drawBitmap(0, yPos, games[i].icon, 16, 16, WHITE);
    }

    arduboy.display();
}

void setup()
{
arduboy.begin();
//initDisplay();      // Инициализация OLED дисплея
//initButtons();      // Инициализация кнопок

arduboy.writeUnitName(unitName);
arduboy.writeShowBootLogoFlag(1);
arduboy.writeShowUnitNameFlag(1);
arduboy.audio.on();
arduboy.audio.saveOnOff();

}


// Главная функция
void loop() {


    while (1) {
        drawMenu(games, selected);

        // Обработка кнопок
        if (arduboy.pressed(UP_BUTTON)) {
            if (selected > 0) selected--;
        }
        if (arduboy.pressed(DOWN_BUTTON)) {
            if (selected < gameCount - 1) selected++;
        }

        // Запуск игры при нажатии A
        if (arduboy.pressed(A_BUTTON)) {
            //uint32_t address = pgm_read_dword(&(games[selected].address));
            launchGame(games[selected].address);
        }

        arduboy.delayShort(150); // Антидребезг
        arduboy.buttonsState();
    }
}

// Переход к игре по адресу в памяти
void launchGame(uint32_t address) {
    // Настройка вектора прерываний

    //write_csr(mstatus, 0);         // Отключение прерываний
    // Переход к игре
    write_csr(mtvec, address); // Можно указать свой обработчик    
        updateCacheLimit(address+0xFFFF);
    goto_game(address);

      while(1);
}

// RAM-function for updating flash cache limit by app size to improve performance
__attribute__((section(".ram_text"))) void updateCacheLimit(uint32_t address)
{
    //uint32_t new_limit = (uintptr_t)__RODATA__ - SPIFI_BASE_ADDRESS;
    uint32_t new_limit = address - SPIFI_BASE_ADDRESS;
    
    // limit cache size by flash size with margin
    //if (new_limit > (FLASH_SIZE - FLASH_END_OFFSET)) new_limit = FLASH_SIZE - FLASH_END_OFFSET;

    uint32_t MCMDbackup = SPIFI_CONFIG->MCMD;           // save current value from MCMD
    SPIFI_CONFIG->STAT |= SPIFI_CONFIG_STAT_RESET_M;    // reset MCMD mode for writing new CLIMIT
    SPIFI_CONFIG->CLIMIT = new_limit;                   // set new CLIMIT
    SPIFI_CONFIG->MCMD = MCMDbackup;                    // restore MCMD value
}

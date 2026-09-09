// Загрузчик игр из SPIFI-памяти. Основан на загрузчике Cathy3K (Arduboy)
#include <Arduboy2.h>
#include <ArduboyFX.h>
Arduboy2 arduboy;

#include "riscv_csr_encoding.h"
#include "csr.h"
#include "mcu32_memory_map.h"
#include "spifi.h"

static bool stk_mode = false;

//#define HEADER_PROGRAM_FACTOR = 128  # Multiply the flashcart slot program length by this
#define HEADER_PROGRAM_FACTOR = 256  # Multiply the flashcart slot program length by this
#define APPLICATION_START_ADDR  0x10000
#define SPIFI_PACKAGE_SIZE 256


//static uint32_t START_IMAGE_OFFSET = 0x020000; // 
#define START_IMAGE_OFFSET  0x00020000


//Формат заголовка (смещения в заголовке слота и длина)
//		обязательные данные
//		//0,7 строка подписи ARDUBOY
        //7 //		1 номер списка (0 экран заголовка загрузчика)
        // 8	//	2 адрес страницы 256 байт предыдущего слота (0xFFFF для первого слота)
        //10	//	2 адрес страницы 256 байт следующего слота
		//2 размера слота на страницах
		//1 размер флэш-памяти приложения в блоках по 256 (было 128) байт (0 — отсутствие)
		//**дополнительные данные**
		//2-страничный адрес данных прикладной программы (0xFFFF отсутствует)
		//Файл данных приложения адреса 2 страницы (0xFFFF отсутствует)
		//2-страничный адрес приложения сохраняет данные (0xFFFF отсутствует)
		//2 Начало EEPROM (0xFFFF none) Дополнительный размер данных приложения (в страницах по 256 байт, обязательно при использовании сохраненных данных)
		//2 Конец EEPROM (0xFFFF отсутствует) Зарезервировано (0xFFFF)
		//32 Подпись приложения SHA256 (рассчитывается по данным шестнадцатеричного файла (до исправления векторов шестнадцатеричного файла), дополненным 0xFF до длины, кратной 256, и данным fx (также дополненным до длины, кратной 256 байтам); для категорий эта область не используется). Эта подпись используется внешними инструментами для быстрой идентификации игры, а не для проверки целостности игры.
		//199 строк UTF-8, завершающихся нулем:


//PROGRAMPAGE_HEADER_INDEX = 15       # "" starting page of program (2 bytes)
//DATAPAGE_HEADER_INDEX = 17          # "" starting page of data (2 bytes)
//SAVEPAGE_HEADER_INDEX = 19          # "" starting page of save (2 bytes)
//DATA_SIZE_HEADER_INDEX = 21         # "" data segment size (2 bytes, factor of 256)
//META_HEADER_INDEX = 57              # "" metadata
//META_HEADER_SIZE = 199              # Length of the metadata section

// Глобальные переменные состояния (аналоги регистров Cathy3K)

// appSize - 1 байт!  внутри Arduboy Tool Set (Chart Editor) используется переменная PROGRAM_SIZE_HEADER_INDEX и константа HEADER_PROGRAM_FACTOR
// предположительно достаточно будет заменить 128 на 256.
// Т.о. можно будет работать с программами, длинной до 64 кбайт. В обычном Arduboy возможно только 32 кбайт. Кроме того, такие программы должны нормально выравниваться по appPage.
// Т.о. теперь мы знаем как делать карту памяти. 

// Кривой подход! при отключении во время записи игры пользователь потеряет загрузчик!
//0-64 кбайт - код игры. 
//Потом можно положить загрузчик (например 16 кбайт). И далее размещать область хранения образа.

// правильный подход:
// в 0- дополнительный загрузчик
// далее собираем все игры под смещенный адрес (например в область 16кб+64 кб)
// далее образ
// если пользователь захочет работать с "голым" - он достаточно компетентен, чтобы восстановить загрузчик потом и поправить адрес сборки в BSP


typedef struct __attribute__((packed)) {
    uint8_t  signature[7];   // "ARDUBOY"
    uint8_t  list;           // номер категории
    uint8_t prevSlotMSB;       // адрес блока 256 байт предыдущего слота, старший байт
    uint8_t prevSlotLSB;        //  адрес блока 256 байт предыдущего слота, младший байт
    uint8_t nextSlotMSB;       // адрес блока 256 байт  следующего слота, старший байт
    uint8_t nextSlotLSB;       // адрес блока 256 байт следующего слота, младший байт
    uint8_t slotSizeMSB;       // размер слота (в 256-байтных страницах)
    uint8_t slotSizeLSB;       // размер слота (в 256-байтных страницах)
    uint8_t appSize;        // размер кода игры (в 256 (было 128)-байтных страницах)
    uint8_t appPageMSB;        // адрес начала кода игры старший байт
    uint8_t appPageLSB;        // адрес начала кода игры младший байт
} FlashBufferObject;
const FlashBufferObject* fbo;


uint8_t  current_list;       // r8 - текущая выбранная категория
//uint8_t  buttons_state;      // r9 - состояние кнопок
uint16_t current_game_page;  // r6:r7 - адрес страницы текущей игры


#define SCROLL_FRAMES 16
#define HSCROLL_STEP  8
// Внешние зависимости (должны быть реализованы в вашем проекте)
uint8_t  ScrollBuffer[1024];     // Буфер для анимации скролла
//uint8_t  buttons_state;          // Состояние кнопок (r9 в оригинале)

extern void SPI_flash_read_addr(uint16_t page_addr);  // Выбор адреса на SPI-флешке
extern void SPI_read_page(uint8_t* buffer);            // Чтение 256 байт
//extern void SPI_flash_deselect(void);                  // Отключение SPI-флешки
//extern void SPI_flash_deselect_display(void);          // + обновление дисплея
//extern void SPI_flash_deselect_display_wait(void);     // + ожидание кадра
//extern void Display(void);                             // Вывод буфера на OLED

#define WIDTH           128
#define HEIGHT          64
#define HSCROLL_STEP    8

// Сигнатура слота
static const char SOFTWARE_IDENTIFIER[7] = {'A', 'R', 'D', 'U', 'B', 'O', 'Y'};





// Статусы возврата LoadApplicationInfo
#define LOAD_STATUS_END_OF_STORAGE  (-1)
#define LOAD_STATUS_NOT_IN_LIST     (1)
#define LOAD_STATUS_LOADED          (0)




// Статусы записи игры
#define BURN_STATUS_OK          0
#define BURN_STATUS_NO_APP      1
#define BURN_STATUS_IN_PROGRESS 2


void setup()
{

    Serial.begin(230400); 


    arduboy.boot(); // увеличиваем скорость загрузки, чтобы нормально работало считывание
    stk_init_image_info();

 
}

// Главная функция
void loop() {
  LoadApplicationInfo(0);
  // Main bootloader loop
  while (1) {

    if (Serial.available()) {
        stk_handle_command();
        return;
    }
    // Handle list selection
    SelectList();
    // Если выбрана категория (current_list != 0) — навигация по играм
        if (current_list != 0) {
            SelectGame();
        } else
        {
            if (arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON)) 
            {
                StartSketch();
            }
        }
        arduboy.delayShort(150); // Антидребезг
        arduboy.buttonsState();
  }

}



// Навигация по категориям (спискам) игр
// При нажатии LEFT/RIGHT переключает current_list на предыдущую/следующую
// категорию. Если нужной категории нет — переходит на ближайшую.
// После вызова current_game_page указывает на первую игру в выбранной категории.
void SelectList(void) {
    int8_t direction = 0;
    
    // === ШАГ 1: Определяем направление по кнопкам ===
    if (arduboy.pressed(LEFT_BUTTON)) {
        direction = -1;
    } else if (arduboy.pressed(RIGHT_BUTTON)) {
        direction = +1;
    } else {
        return;  // Ни LEFT, ни RIGHT не нажаты

    
    }
    
    // === ШАГ 2: Вычисляем номер новой категории ===
    current_list += direction;
    
    // === ШАГ 3: Сканируем все слоты и ищем ближайшую категорию ===
    uint16_t search_addr = 0;
    uint16_t nearest_list_page = 0;
    uint8_t  nearest_list_nr  = 0;
    
    while (true) {

        int8_t status = LoadApplicationInfo(search_addr);
        
        // Конец хранилища — переключаемся на ближайшую категорию
        if (status == LOAD_STATUS_END_OF_STORAGE) {
            break;
        }
        

        // Список найден — выходим с успехом
        if (status == LOAD_STATUS_LOADED) {
           current_game_page = search_addr;  
            return;
        }

        uint8_t slot_list = fbo->list;
        
        // Обновляем "ближайшую категорию"
        if (direction < 0) {
            if (slot_list < current_list) {
                if (slot_list > nearest_list_nr || 
                    (nearest_list_nr == 0 && nearest_list_page == 0)) {
                    nearest_list_page = search_addr;
                    nearest_list_nr  = slot_list;
                }
            }
        } else {
            if (slot_list > current_list) {
                if (slot_list < nearest_list_nr || 
                    (nearest_list_nr == 0 && nearest_list_page == 0)) {
                    nearest_list_page = search_addr;
                    nearest_list_nr  = slot_list;
                }
            }
        }
        
        // Переходим к следующему слоту
        search_addr = ((uint16_t)fbo->nextSlotMSB << 8 | fbo->nextSlotLSB) ;
        if (search_addr == 0) break;
    }
    
    // === ШАГ 4: Переключаемся на найденную категорию ===
    current_list      = nearest_list_nr;
    
    // === ШАГ 5: Ищем первую игру в этой категории ===
    SelectGame_first();
}


//Загружает информацию об игре из слота SPI-флешки 
//page_addr   Адрес страницы слота на SPI-флешке (r6:r7)
// current_list Номер текущего списка/категории (r8)
//return int8_t      LOAD_STATUS_LOADED (0), NOT_IN_LIST (1) или END_OF_STORAGE (-1)
int8_t LoadApplicationInfo(uint16_t page_addr) { // надо переделать: вторым параметром должен быть
    
    // 1. Ищем начало сигнатуры (FBO_SIGNATURE = 0)
    // Если page_addr - это номер страницы (как в Cathy3K), умножаем на 256 для байтового адреса
  
   fbo = (const FlashBufferObject*)(SPIFI_BASE_ADDRESS + START_IMAGE_OFFSET + (page_addr<<8));  // преобразуем в структуру

     if (memcmp(fbo->signature, SOFTWARE_IDENTIFIER, 7) != 0) {
        // Сигнатура неверная — конец хранилища или повреждённый слот
        return LOAD_STATUS_END_OF_STORAGE;
    }

    // === ШАГ 3: Проверяем принадлежность к текущему списку ===
    if (fbo->list != current_list) {
        // Игра есть, но не в выбранной категории — не загружаем дальше
        return LOAD_STATUS_NOT_IN_LIST;
    }
    
    if (!stk_mode) {    
        const uint8_t* title_screen = (const uint8_t*)(SPIFI_BASE_ADDRESS + START_IMAGE_OFFSET +  (page_addr<<8) + 256   );
        
        // Копируем 1024 байт НЕПРЕРЫВНО — именно так, как хранится на флеше!
        memcpy(ScrollBuffer, title_screen, 1024);

        // Читаем 1024 байт последовательно
        //for (int i = 0; i < 1024; i++) {
        //    ScrollBuffer[i] = title_screen[i];
        //}
        // Запускаем анимацию скролла
        scrollTitleScreen();
    }
    return LOAD_STATUS_LOADED;
}

void SelectGame(void) {
    // === ШАГ 1: Определяем направление по кнопкам ===
    if (arduboy.pressed(UP_BUTTON)) {
        SelectGame_up();
        return;
    }
    if (arduboy.pressed(DOWN_BUTTON)) {
        SelectGame_down();
        
        return;
    }
    
    // === ШАГ 2: Нажаты A или B — записываем игру ===
    if (arduboy.pressed(A_BUTTON) || arduboy.pressed(B_BUTTON)) {
        // Проверяем, есть ли что записывать
        if (fbo->appSize == 0) {
            // Игры нет (только титульный экран) — имитируем нажатие DOWN
            SelectGame_down();
            return;
        }
        if (SelectGame_burn()) {
            StartSketch(); // переходим к игре, если запись прошла без ошибок
        } else
        {
            arduboy.setCursor(0, 10);
            arduboy.print(" Сбой копирования"); 
            arduboy.display();
        }
        return;
    }
    
    // Ни одна кнопка не нажата — выходим
}

// ============================================================================
// SelectGame_up - выбор предыдущей игры в категории
// ============================================================================
void SelectGame_up(void) {


    uint8_t  saved_list      = current_list;

    while (true) {
        // Читаем адрес предыдущего слота
        uint16_t prev_addr = ((uint16_t)fbo->prevSlotMSB << 8 | fbo->prevSlotLSB);

        // Если предыдущего слота нет (0xFFFF для первого слота или 0x0000)
        if ((fbo->prevSlotMSB == 0xFF && fbo->prevSlotLSB == 0xFF) || prev_addr == 0) {
            SelectGame_last(saved_list);
            return;
        }

        current_game_page = prev_addr;
        int8_t status = LoadApplicationInfo(current_game_page);
        // Конец хранилища или битый слот — wrap-around к последней игре
        if (status == LOAD_STATUS_END_OF_STORAGE) {
            SelectGame_last(saved_list);
            return;
        }

        // Игра найдена и принадлежит нашему списку — выходим!
        if (status == LOAD_STATUS_LOADED) {
            return; 
        }
        
        // Если status == LOAD_STATUS_NOT_IN_LIST, цикл просто продолжится 
        // и мы перейдём к следующему предыдущему слоту.
    }
}

// ============================================================================
// SelectGame_down - выбор следующей игры в категории
// ============================================================================


void SelectGame_down(void) {
    // В оригинале здесь устанавливался флаг нажатия DOWN для анимации скролла.
    // В C++ мы можем это опустить или передать в функцию отрисовки позже.

    while (true) {
        // Читаем адрес следующего слота
        uint16_t next_addr = ((uint16_t)fbo->nextSlotMSB << 8 | fbo->nextSlotLSB);

        // Если следующего слота нет (обычно 0x0000)
        if (next_addr == 0 || (fbo->nextSlotMSB == 0xFF && fbo->nextSlotLSB == 0xFF)) {
            SelectGame_first();
            return;
        }

        current_game_page = next_addr;

        int8_t status = LoadApplicationInfo(current_game_page);
        // Конец хранилища — wrap-around к первой игре
        if (status == LOAD_STATUS_END_OF_STORAGE) {

            SelectGame_first();
            return;
        }

        // Игра найдена и принадлежит нашему списку — выходим!
        if (status == LOAD_STATUS_LOADED) {

            return; 
        }
        
        // Если status == LOAD_STATUS_NOT_IN_LIST, цикл просто продолжится.
    }
}

// ============================================================================
// SelectGame_first - поиск первой игры в категории (wrap-around)
// ============================================================================
bool SelectGame_first(void) {
    uint16_t search_addr = 0;
    
    while (true) {
       int8_t status = LoadApplicationInfo(search_addr);
        // Конец хранилища — категория пуста
        if (status == LOAD_STATUS_END_OF_STORAGE) {
            return false;
        }
        
        // Игра найдена в нужной категории
        if (status == LOAD_STATUS_LOADED) {
            current_game_page = search_addr;

            return true;
        }
        
        // Переходим к следующему слоту
        search_addr = ((uint16_t)fbo->nextSlotMSB << 8 | fbo->nextSlotLSB);
        if (search_addr == 0) return false;
    }
}

// ============================================================================
// SelectGame_last - поиск последней игры в категории (wrap-around)
// ============================================================================
// Идёт по всем слотам от начала и запоминает последний, принадлежащий нужной категории
void SelectGame_last(uint8_t target_list) {
    uint16_t search_addr =  current_game_page; // ВЕРОЯТНО, ЕСЛИ ЗДЕСЬ ИСПОЛЬЗОВАТЬ АДРЕС текущего выбранного слота, а не 0, то можно сохранить код из menu9 с присвоением категории=0
    uint16_t last_found_page = 0;
    bool     found = false;
    
    // В оригинале: clr r8 — чтобы LoadApplicationInfo не грузила title screens
    uint8_t saved_list = current_list;
    current_list = 0;  // Режим "загрузчика" — без загрузки title screens

    while (true) {
        int8_t status = LoadApplicationInfo(search_addr);
        // Конец хранилища
        if (status == LOAD_STATUS_END_OF_STORAGE) {
            break;
        }
        
        // Проверяем, принадлежит ли слот нужной категории
        if (fbo->list == target_list) {
        
            last_found_page = search_addr;
            found = true;
        }
        
        // Переходим к следующему слоту
        search_addr = ((uint16_t)fbo->nextSlotMSB << 8 | fbo->nextSlotLSB) ;
        if (search_addr == 0) break;
    }
    
    // Восстанавливаем список
   current_list = saved_list;

    if (found) {
        current_game_page = last_found_page;
        // Загружаем информацию о найденной игре (с title screen)
        LoadApplicationInfo(current_game_page);
    }
}



void scrollTitleScreen00() {
    uint8_t* DisplayBuffer = arduboy.getBuffer(); 
    memcpy(DisplayBuffer, ScrollBuffer, 1024);
    arduboy.display();
}


// ============================================================================
// Вертикальный скролл (на 4 пикселя за кадр)
// ============================================================================

void scrollUp() {
    uint8_t* DisplayBuffer = arduboy.getBuffer();

    // 1. Сдвигаем DisplayBuffer вверх на 4 пикселя
    for (int page = 0; page < 7; page++) {
        uint8_t* dst = DisplayBuffer + page * WIDTH;
        uint8_t* src = DisplayBuffer + (page + 1) * WIDTH;
        for (int x = 0; x < WIDTH; x++) {
            // Старые нижние 4 бита уходят наверх, новые приходят из следующей страницы
            dst[x] = (dst[x] >> 4) | (src[x] << 4);
        }
    }


    // 2. Последняя страница DisplayBuffer заполняется из первой страницы ScrollBuffer
    uint8_t* dst = DisplayBuffer + 7 * WIDTH;
    uint8_t* src = ScrollBuffer;
    for (int x = 0; x < WIDTH; x++) {
        dst[x] = (dst[x] >> 4) | (src[x] << 4);
        //dst[x] = src[x];
    } 


       // 3. Сдвигаем сам ScrollBuffer вверх на 4 пикселя (как в оригинале Cathy3K)
    for (int page = 0; page < 7; page++) {
        uint8_t* dst = ScrollBuffer + page * WIDTH;
        uint8_t* src = ScrollBuffer + (page + 1) * WIDTH;
        for (int x = 0; x < WIDTH; x++) {
            dst[x] = (dst[x] >> 4) | (src[x] << 4);
            //dst[x] = (src[x]);
        }
    }
    uint8_t* lastPageSB = ScrollBuffer + 7 * WIDTH;
    for (int x = 0; x < WIDTH; x++) {
        lastPageSB[x] = (lastPageSB[x] >> 4); // Просто сдвигаем вверх, снизу заходят нули (чернота)
    }
}

void scrollDown() {
    uint8_t* DisplayBuffer = arduboy.getBuffer();
    
    // 1. Сдвигаем DisplayBuffer вниз на 4 пикселя
    for (int page = 7; page > 0; page--) {
        uint8_t* dst = DisplayBuffer + page * WIDTH;
        uint8_t* src = DisplayBuffer + (page - 1) * WIDTH;
        for (int x = 0; x < WIDTH; x++) {
            // Старые верхние 4 бита уходят вниз, новые приходят из предыдущей страницы
            dst[x] = (dst[x] << 4) | (src[x] >> 4);
        }
    }
    
    // 2. Первая страница DisplayBuffer заполняется из последней страницы ScrollBuffer
    uint8_t* dst = DisplayBuffer;
    uint8_t* src = ScrollBuffer + 7 * WIDTH;
    for (int x = 0; x < WIDTH; x++) {
        dst[x] = (dst[x] << 4) | (src[x] >> 4);
    }
    
    // 3. Сдвигаем сам ScrollBuffer вниз на 4 пикселя
    for (int page = 7; page > 0; page--) {
        uint8_t* dst = ScrollBuffer + page * WIDTH;
        uint8_t* src = ScrollBuffer + (page - 1) * WIDTH;
        for (int x = 0; x < WIDTH; x++) {
            dst[x] = (dst[x] << 4) | (src[x] >> 4);
        }
    }
    
    uint8_t* firstPageSB = ScrollBuffer;
    for (int x = 0; x < WIDTH; x++) {
        firstPageSB[x] = (firstPageSB[x] << 4); // Просто сдвигаем вверх, снизу заходят нули (чернота)
    }
}

// ============================================================================
// Горизонтальный скролл ВЛЕВО (картинка движется влево, новые данные справа)
// frame: номер кадра (0..15)
// ============================================================================
void scrollLeft(int frame) {
    uint8_t* DisplayBuffer = arduboy.getBuffer();
    
    for (int page = 0; page < 8; page++) {
        uint8_t* dst = DisplayBuffer + page * WIDTH;
        
        // 1. Сдвигаем существующее содержимое страницы влево на 8 байт
        for (int x = 0; x < WIDTH - HSCROLL_STEP; x++) {
            dst[x] = dst[x + HSCROLL_STEP];
        }
        
        // 2. Подгружаем новые 8 байт из ScrollBuffer СПРАВА
        // Для каждой страницы берём данные из её строки в ScrollBuffer
        int src_offset = page * WIDTH + frame * HSCROLL_STEP;
        for (int x = 0; x < HSCROLL_STEP; x++) {
            dst[WIDTH - HSCROLL_STEP + x] = ScrollBuffer[src_offset + x];
        }
    }
}

// ============================================================================
// Горизонтальный скролл ВПРАВО (картинка движется вправо, новые данные слева)
// frame: номер кадра (0..15)
// ============================================================================
void scrollRight(int frame) {
    uint8_t* DisplayBuffer = arduboy.getBuffer();
    
    for (int page = 0; page < 8; page++) {
        uint8_t* dst = DisplayBuffer + page * WIDTH;
        
        // 1. Сдвигаем существующее содержимое страницы вправо на 8 байт
        for (int x = WIDTH - 1; x >= HSCROLL_STEP; x--) {
            dst[x] = dst[x - HSCROLL_STEP];
        }
        
        // 2. Подгружаем новые 8 байт из ScrollBuffer СЛЕВА
        // Берём данные с конца строки и движемся к началу
        int src_offset = page * WIDTH + (WIDTH - HSCROLL_STEP) - frame * HSCROLL_STEP;
        for (int x = 0; x < HSCROLL_STEP; x++) {
            dst[x] = ScrollBuffer[src_offset + x];
        }
    }
}

// ============================================================================
// Главный цикл анимации скролла
// ============================================================================
void scrollTitleScreen() {
    // Определяем направление по нажатым кнопкам
    // В Cathy3K: LEFT_BUTTON → scroll_right (картинка едет вправо)
    //            RIGHT_BUTTON → scroll_left (картинка едет влево)
    enum ScrollDir { SCROLL_LEFT, SCROLL_RIGHT, SCROLL_UP, SCROLL_DOWN };
    ScrollDir dir = SCROLL_LEFT;  // По умолчанию (как в Cathy3K)
    
    if (arduboy.pressed(UP_BUTTON))        dir = SCROLL_DOWN;
    else if (arduboy.pressed(DOWN_BUTTON)) dir = SCROLL_UP;
    else if (arduboy.pressed(LEFT_BUTTON)) dir = SCROLL_RIGHT;
    else if (arduboy.pressed(RIGHT_BUTTON)) dir = SCROLL_LEFT;

    // Анимация из 16 кадров
    for (int frame = 0; frame < SCROLL_FRAMES; frame++) {

        switch (dir) {
            case SCROLL_LEFT:  scrollLeft(frame);  break;
            case SCROLL_RIGHT: scrollRight(frame); break;
            case SCROLL_UP:    scrollUp();         break;
            case SCROLL_DOWN:  scrollDown();       break;
        }
        
        // Выводим буфер на дисплей

                    arduboy.display();   
        // Задержка для плавности анимации
        arduboy.delayShort(50);
    }
}


bool SelectGame_burn(void) {

    // Адрес начала кода игры на "SPI flash" (в 256-байтных единицах - переводим в байты)
    uint16_t app_flash_page = ((uint16_t)fbo->appPageMSB << 8 | fbo->appPageLSB) ;
    uint24_t app_flash_addr = ((uint24_t)app_flash_page) << 8 ;



    uint8_t app_page_size256 = fbo->appSize;

   uint24_t app_size = ((uint24_t)(app_page_size256))<< 8;


    //Цикл записи/очистки по 256. Считаем, что читаемые и записываемые данные выровнены по 256 и по 4к
    // по хорошему нужно добавить проверку?
    
    const uint8_t* src_start = (const uint8_t*)(SPIFI_BASE_ADDRESS + START_IMAGE_OFFSET + app_flash_addr);
    const uint8_t* dst_start = (const uint8_t*)(SPIFI_BASE_ADDRESS+APPLICATION_START_ADDR);
    


    uint8_t buffer_part[256];

    for (uint16_t write_page256 = 0; write_page256 < app_page_size256; write_page256++) {


     if ((write_page256 % 16) == 0) {boot_page_erase(write_page256); } // если начались новые 16 страниц размером 256 байт - это новый сектор в 4096 байта. Стираем его
    uint32_t offset = (uint32_t)write_page256 * SPIFI_PACKAGE_SIZE;

        const uint8_t* src_addr = src_start + offset;
        memcpy(buffer_part, src_addr, SPIFI_PACKAGE_SIZE);


        boot_page_write(write_page256, (uint8_t*)buffer_part);
        
        // Указатель на физическое место во флеш-памяти, куда только что записали данные
        const uint8_t* flash_part = (const uint8_t*)(dst_start + offset);
        
        if (memcmp(buffer_part, flash_part, SPIFI_PACKAGE_SIZE) != 0) {
            return false;
        }
    drawProgressBar(write_page256, app_page_size256);

    }

return true;
}


__attribute__((section(".ram_text"))) void  boot_page_erase(uint16_t page)
{
 
// ПЕРЕПРОВЕРЬ ВНУТРИ ВЫРАВНИВАНИЕ ПО 4к !!!! (uint24_t)(page)) << 8) <- выравнивает по 256 !!!!. Для выравнивая по 4к нужно смещать на << 12, но тогда номера страниц будут другие! 
// в аналогичной функции FX, по хорошему, вероятно, нужно дополнительно проверять выравнивание по 4к !!!!!!!!

	uint32_t MCMDbackup; 
	uint32_t CLIMITbackup;           // 
	//EPIC->MASK_LEVEL_CLEAR = HAL_EPIC_TIMER32_1_MASK ; // отключаем прерывания по уровню (вывод звука)
	EPIC->MASK_LEVEL_CLEAR = HAL_EPIC_TIMER16_1_MASK; // пока сделано только для SPIBEAR
    FX::enableCMD(&CLIMITbackup,&MCMDbackup);
	FX::writeEnable();
	//стирание
	FX::my_SPIFI_SendCommand_LL(cmd_erase_4k_qpi, (((uint24_t)(page)) << 8) + APPLICATION_START_ADDR+ SPIFI_BASE_ADDRESS, 0, 0, 0, 0, HAL_SPIFI_TIMEOUT);
	FX::waitWhileBusy();
	FX::disableCMD(CLIMITbackup,MCMDbackup);
	//EPIC->MASK_LEVEL_SET = HAL_EPIC_TIMER32_1_MASK ; // отключаем прерывания по уровню (вывод звука)
    EPIC->MASK_LEVEL_SET = HAL_EPIC_TIMER16_1_MASK;
}

__attribute__((section(".ram_text"))) void boot_page_write(uint16_t page, uint8_t* buffer)
{
	uint32_t MCMDbackup; 
	uint32_t CLIMITbackup;           // 
	FX::enableCMD(&CLIMITbackup,&MCMDbackup);
	FX::writeEnable();
	FX::my_SPIFI_SendCommand_LL(cmd_write_bytes_qpi, (((uint24_t)(page)) << 8)+ APPLICATION_START_ADDR+ SPIFI_BASE_ADDRESS, SPIFI_PACKAGE_SIZE, 0, buffer, 0, HAL_SPIFI_TIMEOUT); // SPIFI_PACKAGE_SIZE =256 Размер буфера
	FX::waitWhileBusy();
	FX::disableCMD(CLIMITbackup,MCMDbackup);
}

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

__attribute__((section(".ram_text"))) void StartSketch() {
    // Настройка вектора прерываний
    //write_csr(mstatus, 0);         // Отключение прерываний
    // Переход к игре
    const uint8_t* dst_start = (const uint8_t*)(SPIFI_BASE_ADDRESS+APPLICATION_START_ADDR);
    if (dst_start[0]==0xFF && dst_start[1]==0xFF) return;

    write_csr(mtvec, (uint32_t)dst_start);    
    updateCacheLimit((uint32_t)dst_start+0xFFFF); // +64 кбайт
    asm volatile (
        "mv   ra, %[addr]\n\t"  // Перемещаем переданный адрес в регистр ra
        "jalr ra"               // Переходим по адресу, используя jalr
        :
        : [addr] "r" (dst_start)  // Используем регистр для передачи адреса
        : "ra"                  // Указываем, что регистр ra будет изменен
    );

      while(1);
}



void drawProgressBar(uint8_t current, uint8_t total) {
    uint8_t* buf = arduboy.getBuffer();
    
    const uint8_t page = 3;
    const uint8_t start_col = 29;
    const uint8_t bar_steps = 64;
    
    uint8_t filled_steps = (uint16_t)bar_steps * current / total;
    uint16_t buf_pos = page * WIDTH + start_col;
    
    buf[buf_pos++] = 0x00;
    buf[buf_pos++] = 0x7E;
    
    for (uint8_t i = 0; i < bar_steps; i++) {
        buf[buf_pos++] = (i < filled_steps) ? 0x5A : 0x42;
    }
    
    buf[buf_pos++] = 0x7E;
    buf[buf_pos] = 0x00;
    
 
    arduboy.display();
}

// ============================================================================
// Обработчик протокола STK500v2 для совместимости с arduboy-tools
// ============================================================================

// Глобальное состояние протокола
static uint32_t stk_address = 0;           // Текущий адрес в байтах
static uint16_t stk_image_pages = 0;       // Количество страниц в образе (вычисляется при старте)

// Определяем размер образа при инициализации
void stk_init_image_info() {
    stk_image_pages = 0;
    uint32_t offset = 0;
    const uint8_t* flash_base = (const uint8_t*)(SPIFI_BASE_ADDRESS + START_IMAGE_OFFSET);
    
    uint16_t slot_count = 0;
    
    while (offset < ((8 * 1024 * 1024)-START_IMAGE_OFFSET) ) {
        const FlashBufferObject* slot = (const FlashBufferObject*)(flash_base + offset);
        
        if (memcmp(slot->signature, SOFTWARE_IDENTIFIER, 7) != 0) {
            break;
        }
        
        uint32_t slot_size = ((uint16_t)slot->slotSizeMSB << 8 | slot->slotSizeLSB) << 8;


        if (slot_size == 0) {
            break;
        }
    
        slot_count++;
        offset += slot_size;
    }
    
    // 2. ОКРУГЛЯЕМ ВВЕРХ до границы 64 КБ (65536 байт = 0x10000)
    // Формула: (value + 0xFFFF) & ~0xFFFF
    offset = (offset + 0xFFFF) & ~0xFFFF;

    stk_image_pages = offset / 256;

}


/*
void stk_init_image_info() {
    stk_image_pages = 0;
    uint16_t current_page = 0;
    
    // Сканируем слоты, используя навигационные ссылки (как в меню)
    while (true) {
        // Получаем указатель на текущий слот
        const FlashBufferObject* slot = (const FlashBufferObject*)(
            SPIFI_BASE_ADDRESS + START_IMAGE_OFFSET + (current_page << 8)
        );
        
        // Проверяем сигнатуру
        if (memcmp(slot->signature, SOFTWARE_IDENTIFIER, 7) != 0) {
            break;  // Конец образа
        }
        
        // Получаем размер слота в страницах
        uint16_t slot_size_pages = slot->slotSize;
        if (slot_size_pages == 0) {
            break;  // Неверный размер
        }
        
        // Добавляем размер слота к общему размеру
        stk_image_pages += slot_size_pages;
        
        // Получаем адрес следующего слота
        uint16_t next_page = ((uint16_t)slot->nextSlotMSB << 8) | slot->nextSlotLSB;
        
        // Если следующего слота нет — конец образа
        if (next_page == 0 || (slot->nextSlotMSB == 0xFF && slot->nextSlotLSB == 0xFF)) {
            break;
        }
        
        // Переходим к следующему слоту
        current_page = next_page;
    }
    
    // Serial.print("STK Image size: ");
    // Serial.print(stk_image_pages);
    // Serial.println(" pages");
}
*/


// Главный обработчик команд
void stk_handle_command() {
    if (!Serial.available()) return;
    
    stk_mode = true;  // Включаем режим быстрой обработки
    
    char cmd = Serial.read();
    
    if (cmd == '\r' || cmd == '\n' || cmd == ' ') {
        return;
    }
    
    switch (cmd) {
        case 'V':
            Serial.print("15");
            Serial.flush();
            break;
            
        case 'j':
            Serial.write((stk_image_pages >> 8) & 0xFF);
            Serial.write(stk_image_pages & 0xFF);
            Serial.write((uint8_t)0x00);
            Serial.flush();
            break;
            
        case 'A':
            while (Serial.available() < 2) {}  // Ждём оба байта
            {
                uint8_t addr_hi = Serial.read();
                uint8_t addr_lo = Serial.read();
                stk_address = ((uint32_t)addr_hi << 8 | addr_lo) << 8;
            }
            Serial.write('\r');
            Serial.flush();


            break;

case 'g':  // Read block
    while (Serial.available() < 3) {}
    {
        uint8_t len_hi = Serial.read();
        uint8_t len_lo = Serial.read();
        char mem_type = Serial.read();
        



        //  0 означает 65536 (конвенция STK500v2)
        uint32_t block_len = ((uint16_t)len_hi << 8) | len_lo;
        if (block_len == 0) block_len = 65536;
        


        if (mem_type == 'C') {
            const uint8_t* flash_ptr = (const uint8_t*)(
                SPIFI_BASE_ADDRESS + START_IMAGE_OFFSET + stk_address
            );
            uint32_t max_size = (uint32_t)stk_image_pages * 256;
            
            if (stk_address + block_len > max_size) {
                uint32_t valid_len = (stk_address < max_size) ? 
                                     (max_size - stk_address) : 0;
                for (uint32_t i = 0; i < valid_len; i++) {
                    Serial.write(flash_ptr[i]);
                }
                for (uint32_t i = 0; i < block_len - valid_len; i++) {
                    Serial.write(0xFF);
                }
           

           
            } else {
                //  Отправляем по частям, чтобы не переполнить USB-буфер 
                uint32_t remaining = block_len;
                const uint8_t* ptr = flash_ptr;
                while (remaining > 0) {
                    uint32_t chunk = (remaining > 4096) ? 4096 : remaining;
                    Serial.write(ptr, chunk);
                    Serial.flush();
                    ptr += chunk;
                    remaining -= chunk;
                }
            }
            stk_address += block_len;
        }
        // ... остальные типы памяти ...
    }
    //Serial.write('\r'); // Нет передачи символа \r !
    //Serial.flush();
    break;

        case 'x':
            while (!Serial.available()) {}  //  ВСЕГДА ждём параметр!
            Serial.read();  // читаем и игнорируем
            Serial.write('\r');
            Serial.flush();
            break;            
        case 'E':
            Serial.write('\r');
            Serial.flush();
            break;
            
        case 'S':
            Serial.write("ARDUBOY");
            Serial.flush();
            break;
            
        default:
            // Неизвестная команда - игнорируем, но можно добавить отладочный вывод
            break;
    }
}
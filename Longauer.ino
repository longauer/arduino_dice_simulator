#include "funshield.h"

// constant variables

constexpr int dice_sizes[] = {4, 6, 8, 10, 12, 20, 100};
constexpr int num_of_throws[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
constexpr int powers_of_10[] {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000};

// map of letter glyphs
constexpr byte LETTER_GLYPH[] {
  0b10001000,   // A
  0b10000011,   // b
  0b11000110,   // C
  0b10100001,   // d
  0b10000110,   // E
  0b10001110,   // F
  0b10000010,   // G
  0b10001001,   // H
  0b11111001,   // I
  0b11100001,   // J
  0b10000101,   // K
  0b11000111,   // L
  0b11001000,   // M
  0b10101011,   // n
  0b10100011,   // o
  0b10001100,   // P
  0b10011000,   // q
  0b10101111,   // r
  0b10010010,   // S
  0b10000111,   // t
  0b11000001,   // U
  0b11100011,   // v
  0b10000001,   // W
  0b10110110,   // ksi
  0b10010001,   // Y
  0b10100100,   // Z
};
constexpr byte EMPTY_GLYPH = 0b11111111;

constexpr int positionsCount = 4;


inline int power_10(int p){
  return powers_of_10[p];
}

void writeGlyphBitmask(byte glyph, byte pos_bitmask){
  digitalWrite(latch_pin, LOW);
  shiftOut(data_pin, clock_pin, MSBFIRST, glyph);
  shiftOut(data_pin, clock_pin, MSBFIRST, pos_bitmask);
  digitalWrite(latch_pin, HIGH);
}

class Display;
class Button;
class Animation;


class Animation{
public:

  Animation(){
    horizontal_bars_rates = new int[4]{80, 80, 80, 80};
    horizontal_xor = new byte[3]{0b01001000, 0b00001001, 0b01000001};
    horizontal_stage = new int[4]{0, 1, 2, 3};
    horizontal_last_time = new unsigned long[4]{0, 0, 0, 0};
    GLYPHS = new byte[positionsCount];
    for (int i = 0; i<positionsCount; i++){
      GLYPHS[i] = EMPTY_GLYPH;
    }

    // initiatialize the pattern
    GLYPHS[3]^=0b00110000;
  }

  // animation loop + multiplexing
  void loop(){
    unsigned long current_time = millis();

    if (current_time - UV_last_time >= upper_vertical_rate){
      UV_last_time = current_time;
      move_UV();
    }

    if (current_time - LV_last_time >= lower_vertical_rate){
      LV_last_time = current_time;
      move_LV();
    }

    for (int i = 0; i<positionsCount; i++){
      if (current_time - horizontal_last_time[i] >= horizontal_bars_rates[i]){
        horizontal_last_time[i] = current_time;
        move_horizontal(i);
      }
    }

    writeGlyphBitmask(GLYPHS[display_pos], 1<<(positionsCount - display_pos - 1));
    display_pos = (display_pos + 1)%positionsCount;
  }

  ~Animation() {
    // Deallocate memory in the destructor
    delete[] horizontal_bars_rates;
    delete[] horizontal_xor;
    delete[] horizontal_stage;
    delete[] horizontal_last_time;
  }

private:
  byte* GLYPHS;

  int* horizontal_bars_rates;
  int upper_vertical_rate = 100; 
  int lower_vertical_rate = 100;

  int last_UV_pos = 7;
  int last_LV_pos = 7;
  int LV_dir = 1;
  int UV_dir = 1;

  int* horizontal_stage;
  byte* horizontal_xor;
  byte horizontal_and = 0b10110110;

  // timers
  unsigned long UV_last_time = 0;
  unsigned long LV_last_time = 0;
  unsigned long* horizontal_last_time;
  
  int display_pos = 0;

  void move_UV(){
    //int curr_UV_pos = (last_UV_pos+7)%8;
    if (last_UV_pos == 0 || last_UV_pos == 7) UV_dir*=-1;
    int curr_UV_pos = last_UV_pos+UV_dir;

    if (UV_dir == -1){
      if (curr_UV_pos%2==0){
        GLYPHS[last_UV_pos/2]^=0b00100010;
      }else{
        GLYPHS[last_UV_pos/2]^=0b00000010;
        GLYPHS[curr_UV_pos/2]^=0b00100000;
      }
    }else{
      if (curr_UV_pos%2==1){
        GLYPHS[last_UV_pos/2]^=0b00100010;
      }else{
        GLYPHS[last_UV_pos/2]^=0b00100000;
        GLYPHS[curr_UV_pos/2]^=0b00000010;
      }
    }
    last_UV_pos = curr_UV_pos;
  }

  void move_LV(){
    //int curr_LV_pos = (last_LV_pos+7)%8;
    if (last_LV_pos == 0 || last_LV_pos == 7) LV_dir*=-1;
    int curr_LV_pos = last_LV_pos+LV_dir;

    if (LV_dir == -1){
      if (curr_LV_pos%2==0){
        GLYPHS[last_LV_pos/2]^=0b00010100;
      }else{
        GLYPHS[last_LV_pos/2]^=0b00000100;
        GLYPHS[curr_LV_pos/2]^=0b00010000;
      }
    }else{
      if (curr_LV_pos%2==1){
        GLYPHS[last_LV_pos/2]^=0b00010100;
      }else{
        GLYPHS[last_LV_pos/2]^=0b00010000;
        GLYPHS[curr_LV_pos/2]^=0b00000100;
      }
    }

    last_LV_pos = curr_LV_pos;
  }

  void move_horizontal(int pos){
    GLYPHS[pos]&=horizontal_and;
    GLYPHS[pos]^=horizontal_xor[horizontal_stage[pos]];
    horizontal_stage[pos] = (horizontal_stage[pos]+1)%3;
  }

};


class Display{
public:

  Display()=default;

  void setup(){
    pinMode(latch_pin, OUTPUT);
    pinMode(clock_pin, OUTPUT);
    pinMode(data_pin, OUTPUT);
  }

  void set_dice_size(int size){
    dice_size_ = size;
  }

  void set_num_throws(int num_throws){
    num_throws_ = num_throws;
  }

  void show(){
   
    switch (position_){
      case 2:
        writeGlyphBitmask(LETTER_GLYPH['d' - 'a'] , 1<<(positionsCount - position_ - 1));
        break;
      case 3:
        writeGlyphBitmask(digits[num_throws_] , 1<<(positionsCount - position_ - 1));
        break;
      case 1:
        writeGlyphBitmask(digits[(dice_size_/10)%10] , 1<<(positionsCount - position_ - 1));
          break;
      case 0:
        writeGlyphBitmask(digits[(dice_size_)%10] , 1<<(positionsCount - position_ - 1));
        break;
    }

    position_ = (position_+ positionsCount - 1)%positionsCount;
  }

  void show_throw(int thr, int first_non_zero){
    int digit = (thr/power_10(position_))%10;
    if (position_ < first_non_zero) writeGlyphBitmask(digits[digit], 1<<(positionsCount - position_ - 1));
    else writeGlyphBitmask(EMPTY_GLYPH, 1<<(positionsCount - position_ - 1));
    position_ = (position_+ positionsCount - 1)%positionsCount;
  }

  void show_animation(){
    animation.loop();
  }


private:
  byte position_;
  int num_throws_;
  int dice_size_;
  Animation animation;
};


class Button{
public:

  enum State{released, pressed};

  unsigned long last_time_pressed;

  Button()=default;

  void setup(int pin_init){
    button_pin_= pin_init;
    pinMode(button_pin_, INPUT);  
  }

  bool activated(){
    state_ = (get_state() == ON ? pressed : released);
    if (state_ == pressed && previous_state_ == released){
      previous_state_ = state_;
      return true;
    }
    previous_state_ = state_;
    return false;
  }

  bool is_pressed(){
    state_ = (get_state() == ON ? pressed : released);
    if (previous_state_ == released && state_ == pressed){
      previous_state_ = state_;
      last_time_pressed = millis();
    }
    previous_state_ = state_;
    return (state_ == pressed ? true : released);
  }

  bool is_released(){
    state_ = (get_state() == ON ? pressed : released);
    previous_state_ = state_;
    return (state_ == pressed ? true : released);
  }

  int get_state(){
    return digitalRead(button_pin_);
  }

private:
  State state_;
  State previous_state_;
  int button_pin_;
};


class RandomThrowGenerator{

    enum State{throw_generation, dice_config, throws_config};

private:
  Button generate_throws_;
  Button throw_num_adjust_;
  Button dice_type_;
  Display display_;
  int num_of_throws_;
  int dice_size_;
  unsigned long current_throw_sum_;
  State mode;

  // generates a random number in the range 0 - 99
  int random_number(){
    int first_digit = ((micros()/100)%10)*10;
    int second_digit = (micros()/10)%10;
    return first_digit + second_digit;
  }

  // generates random number in the range 1 to max_val
  int random_in_range(int max_val){
    int rand_num = (int)(random_number()*max_val/100);
    return rand_num+1;
  }

  // helper function for hiding leading zeros of the numbers thrown
  int first_non_zero(int th){
    int cnt = positionsCount;
    while(th){
      th/=10;
      cnt--;
    }
    return positionsCount-cnt;
  }

  void throw_generation_func(){
    if (generate_throws_.is_pressed()){
      current_throw_sum_ = 0;
      for (int i = 0; i<num_of_throws[num_of_throws_]; i++){
        current_throw_sum_ += random_in_range(dice_sizes[dice_size_]);
      }
      display_.show_animation();
    }else if (throw_num_adjust_.activated()){
      mode = throws_config;
    }else if (dice_type_.activated()){
      mode = dice_config;
    }else{
      display_.show_throw(current_throw_sum_, first_non_zero(current_throw_sum_));
    }
  }

  void dice_config_func(){
    if (dice_type_.activated()){
      dice_size_ = (dice_size_+1)%(sizeof(dice_sizes)/sizeof(dice_sizes[0]));
      display_.set_dice_size(dice_sizes[dice_size_]);
    }else if (throw_num_adjust_.activated()){
      mode = throws_config;
    }else if (generate_throws_.activated()){
      mode = throw_generation;
    }
    display_.show();
  }

  void throws_config_func(){
    if (throw_num_adjust_.activated()){
      num_of_throws_ = (num_of_throws_+1)%(sizeof(num_of_throws)/sizeof(num_of_throws[0]));
      display_.set_num_throws(num_of_throws[num_of_throws_]);
    }else if (dice_type_.activated()){
      mode = dice_config;
    }else if (generate_throws_.activated()){
      mode = throw_generation;
    }
    display_.show();
  }

public:

  RandomThrowGenerator()=default;

  void setup(){
    num_of_throws_ = 0;
    dice_size_ = 0;

    generate_throws_.setup(button1_pin);
    throw_num_adjust_.setup(button2_pin);
    dice_type_.setup(button3_pin);

    display_.setup();
    display_.set_num_throws(num_of_throws[num_of_throws_]);
    display_.set_dice_size(dice_sizes[dice_size_]);

    current_throw_sum_ = 0;
    mode = dice_config;
  }

  void loop(){

    switch (mode){
      case throw_generation:
        throw_generation_func();
        break;
      case dice_config:
        dice_config_func();
        break;
      case throws_config:
        throws_config_func();
        break;
    }
  }

};

RandomThrowGenerator random_throw_gener;

void setup() {

  // Serial.begin(9600);
  random_throw_gener.setup();

}

void loop() {

  random_throw_gener.loop();

}
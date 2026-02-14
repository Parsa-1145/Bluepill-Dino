#include "Animations.h"
#include "Adafruit_SSD1306.h"
#include "Adafruit_GFX.h"

int SCR_W = 128, SCR_H = 64;
Adafruit_SSD1306 display(SCR_W, SCR_H, &Wire, -1);

struct Player{
  float pos[2]  = {10, 0};
  float size[2] = {20, 21};
  float vel[2]  = {10, 0};
  float acl[2]  = {0, 0}; //acceleration
  bool inAir = false;
  float animTime;
};

struct Game{
  float deltaTime;
  float score = 0;
  int scoreInt = 0;
  float time = 0;
  float gravity = 80 * 4;
  float jumpVel = 60 * 2;
  float groundPos = 2;
  Player player;
};

enum ProgramState{
  PreGame,
  GameEnd,
  InGame
};

// Forward Refs
void onButton();

// Variables
Game* game;
ProgramState programState = PreGame;
long long prevTime, curTime;
float deltaTime;

const char* prevGameText = "Press to start";
float prevGameTextX, prevGameTextY;

// Util
void blinkLed(int milis){
  digitalWrite(PC13, LOW);
  delay(milis);
  digitalWrite(PC13, HIGH);
  delay(milis);
}
void blinkLed(int milis, int num){
  for(int i = 0; i < num; i++){
    blinkLed(milis);
  }
}
int16_t getTextWidth(const char* str, uint8_t textSize) {
  int16_t x1, y1;
  uint16_t w, h;

  display.setTextSize(textSize);
  display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
  return w;
}
int16_t getTextHeight(const char* str, uint8_t textSize) {
  int16_t x1, y1;
  uint16_t w, h;

  display.setTextSize(textSize);
  display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
  return h;
}

// Setup
void setupDisplay(){
  Wire.begin();
  Wire.setClock(400000);
  while(!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)){
    blinkLed(200, 3);
    delay(1000);
  }
  display.clearDisplay();
}


void setup(){
  pinMode(PC13, OUTPUT);
  digitalWrite(PC13, HIGH);
    blinkLed(200, 3);

  setupDisplay();

  prevGameTextX = (SCR_W - getTextWidth (prevGameText, 1)) / 2;
  prevGameTextY = (SCR_H - getTextHeight(prevGameText, 1)) / 2;

  attachInterrupt(digitalPinToInterrupt(PB3), onButton, FALLING);

  prevTime = millis();
}


// Game loop

volatile bool btnEvent = false;
volatile uint32_t lastMs = 0;

void onButton() {
  uint32_t now = millis();
  if (now - lastMs > 30) {        // ~30 ms debounce
    lastMs = now;
    btnEvent = true;
  }
}

inline bool wasBtnPressed(){
  if(btnEvent){
    btnEvent = false;
    return true;
  }
  return false;
}

void updateGame(){
  game->player.inAir = (game->player.pos[1] > 1) || (game->player.vel[1] >= 1);

  game->player.pos[1] = (game->player.inAir) * (game->player.pos[1] + game->player.vel[1] * deltaTime); 
  game->player.vel[1] = (game->player.inAir * (game->player.vel[1] - deltaTime * game->gravity)) + (!game->player.inAir) * wasBtnPressed() * game->jumpVel;

  game->time += deltaTime;
  game->score += game->player.vel[0] * deltaTime;
  game->player.vel[0] += deltaTime * 1;
  game->scoreInt = (int) game->score;
}

void render(){
  // player
  if(game->player.inAir){
    display.drawBitmap(game->player.pos[0], SCR_H - (game->player.pos[1] + game->groundPos + game->player.size[1]), jumpAnim_packed, 20, 21, SSD1306_WHITE);
  }else{
    int frame = ((int)( game->time * game->player.vel[0] / 10)) & 1;
    display.drawBitmap(game->player.pos[0], SCR_H - (game->player.pos[1] + game->groundPos + game->player.size[1]), walkAnim_packed[frame], 20, 21, SSD1306_WHITE);
  }

  // display.drawRect(game->player.pos[0], SCR_H - (game->player.pos[1] + game->groundPos + game->player.size[1]), game->player.size[0], game->player.size[1], SSD1306_WHITE);
  display.drawLine(0, SCR_H - game->groundPos, SCR_W, SCR_H - game->groundPos, SSD1306_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(1);
  display.print(game->scoreInt);
}

void startGame(){
  if(game != nullptr){
    free(game);
  }

  game = new Game();

  programState = InGame;
}

void loop(){
  curTime = millis();
  deltaTime = (curTime - prevTime) / 1000.0f;
  prevTime = curTime;

  display.clearDisplay();
  switch (programState) {
    case PreGame:
      if(wasBtnPressed()){
        startGame();
        break;
      }
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(prevGameTextX, prevGameTextY);
      display.print(prevGameText);
      break;
    case GameEnd:
      break;
    case InGame:
      updateGame();
      render();
      break;
  }
  display.display();
}
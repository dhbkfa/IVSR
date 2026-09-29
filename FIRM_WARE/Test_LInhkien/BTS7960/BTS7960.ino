int R_EN = 7;
int L_EN = 8;
int RPWM = 5;
int LPWM = 6;
#define PWM_FREQ 20000
#define PWM_RES  10
void setup() {
  pinMode(R_EN,OUTPUT);
  pinMode(L_EN,OUTPUT);
   pinMode(RPWM,OUTPUT);
  pinMode(LPWM,OUTPUT);
  // ledcAttach(RPWM, PWM_FREQ, PWM_RES);
  // ledcAttach(LPWM, PWM_FREQ, PWM_RES);
  digitalWrite(R_EN,HIGH);
  digitalWrite(L_EN,HIGH);



}

void loop() {
  // Quay thuận 50%
  // ledcWrite(RPWM, 10);
  // ledcWrite(LPWM, 0);
  digitalWrite(RPWM,HIGH);
  digitalWrite(LPWM,0);
  delay(500);

}

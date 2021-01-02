void setup() {
  while (!Serial);
  Serial.begin(115200);
  init_fona();
  //Serial.println("init done");
  String num=get_num();
  String msg="Thank you!";
  send_sms(num,msg);
}

void loop() {
  // put your main code here, to run repeatedly:

}

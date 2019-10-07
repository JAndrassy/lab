#include "SerialRPC.h"

byte frmtBuff[42];

void setup() {

  Serial.begin(115200);

  float x = 0.7;
  int arr[] = {1000, 200, 3000, 4000, 5000, 6000, 7000};
  call(0, "iiisXiiIi", 1, 'x', &x, "xyz", frmtBuff, 5, &x, arr, sizeof(arr) / sizeof(int));
//  call(0, "blvvs", "xiiii", buff, 5, 1, 'x', "xyz");
//  call(0, "vsblv", "iixii", 'x', "xyz", buff, 5, 1);
//  call(0, "vblsv", "iixii", 'x', buff, 5, "xyz", 1);
}

void loop() {
}

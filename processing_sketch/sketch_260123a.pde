import processing.serial.*;

Serial myPort;
float roll, pitch, yaw;

void setup() {
  fullScreen(P3D);

  println(Serial.list());
  myPort = new Serial(this, Serial.list()[1], 115200);
  myPort.bufferUntil('\n');
}

void draw() {
  background(30);
  lights();

  translate(width/2, height/2, 0);

rotateY(HALF_PI);
// Aircraft-style rotation order: yaw → pitch → roll
rotateY(radians(yaw));        // yaw (heading)
rotateX(-radians(pitch));     // pitch (nose up/down)
rotateZ(radians(roll));       // roll (wings)

  drawPlane();
}

void drawPlane() {
  noStroke();
  fill(200, 80, 80);

  // Fuselage
  pushMatrix();
  box(400, 40, 60);
  popMatrix();

  // Wings
  pushMatrix();
  box(120, 10, 600);
  popMatrix();

  // Horizontal tail
  pushMatrix();
  translate(-170, 0, 0);
  box(80, 8, 200);
  popMatrix();

  // Vertical tail
  pushMatrix();
  translate(-170, -30, 0);
  box(10, 80, 100);
  popMatrix();
}

void serialEvent(Serial myPort) {
  String data = myPort.readStringUntil('\n');
  if (data != null) {
    data = trim(data);
    String[] parts = split(data, '/');
    if (parts.length == 3) {
      roll = float(parts[0]);
      pitch = float(parts[1]);
      yaw = float(parts[2]);
    }
  }
}

# ESP32-Object-Recognition
My humble attempt at getting tensorflow and the ESP32-CAM to cooperate (among other things)

## Notes:
**"Webcam_detection.py" isn't plug and play. If you plan on running it, you probably know but you're doing but here's a couple of pointers:**
  * Get [OpenCV](https://www.lfd.uci.edu/~gohlke/pythonlibs/) & [TensorFlow](https://github.com/tensorflow/models/blob/master/research/object_detection/g3doc/tf1_detection_zoo.md)
  * My project uses TensorFlow 1.0 which only support Python <= 3.7
  * Get a TensorFlow model [COCO]

**Similarly, the ESP32 project isn't plug and play either:**
  * Open the arduino code in PlatformIO as that **should** automatically resolve any library dependencies.
  * Change it accordingly
    * Change the ssid/password
    * Set the correct resistors used for the voltage divider
    * Set server port/local ip
  * Power is supplied through a 4.2 lithium ion battery powered through the appropriate regulator

**Web:**
  * Set ESP module's local IP
  * Try to make the webpage (index.html) look nice??



I probably missed stuff, idk

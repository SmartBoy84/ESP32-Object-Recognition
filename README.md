# ESP32-Object-Recognition
My humble attempt at getting tensorflow and the ESP32-CAM to cooperate (among other things)

## Notes:
* "webcam_detection.py" isn't plug and play. If you plan on running it, you probably know but you're doing but here's a couple of pointers
  * Get [OpenCV](https://www.lfd.uci.edu/~gohlke/pythonlibs/) & [TensorFlow](https://github.com/tensorflow/models/blob/master/research/object_detection/g3doc/tf1_detection_zoo.md)
  * My project uses TensorFlow 1.0 which only support Python <= 3.7
  * Get a TensorFlow model [COCO]

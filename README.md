# ESP32-Object-Recognition

My humble attempt at getting tensorflow and the ESP32-CAM to cooperate (among other things)

Notes:
* "webcam_detection.py" isn't plug and play. If you plan on running it, you probably know but you're doing but here's a couple of pointers
  * Get [OpenCV](https://www.lfd.uci.edu/~gohlke/pythonlibs/) & TensorFlow
  * Get a TensorFlow model [COCO]
  * Setup a ESP32 web stream [MJPEG] and change the script to match it's local IP
  * Get a couple of other libraries

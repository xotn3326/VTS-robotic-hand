# Detect hand joints using RGB camera

I use MediaPipe provided by Google for detecting hand-joints and with these joints, write the algorithm to classify the defree of folding of a finger through the relationshop between joints.

![image](https://user-images.githubusercontent.com/70706751/145206953-bd5e205b-2053-43e4-8059-10adfc9b786c.png)


## Requirements

```
conda env create VTS-hand python=3.8
conda activate VTS-hand
pip install -r requirements.txt
```

- 1 RGB camera
- boards for serial communication
  
## Evaluate

For communicating the serial with the board, 
  python finger_fold_5way.py
  
For only detecting hand joints,
  python only_camera.py


## Contributor

Tae-Soo Jeong, B.S. candidate in the Department of Mechanical, Robotics and Energy Engineering, Dongguk University.

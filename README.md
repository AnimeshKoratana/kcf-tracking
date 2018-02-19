# Bus Tracking 
Here we use a KCF (Kernelized Correlation Filters) tracker implemented by OpenCV to track busses from CCTV footage given a dataset. 

Requirements before you run:
1. make and cmake
2. FFMPEG with libx264 
3. [OpenCV 3.4](https://github.com/opencv/opencv) with FFmpeg bindings (make sure to compile with the [opencv_contrib modules](https://github.com/opencv/opencv_contrib))
4. [Facebook Folly](https://github.com/facebook/folly)
  * [Glog](https://github.com/google/glog)
  * [Double Conversion](https://github.com/google/double-conversion)

To run:
```
cmake CMakeLists.txt        
make
./bus_tracking          #Run the program
```



**Outputs:**

The script will read the dataset (too large to upload to the git repo) and find the first instance of each bus. Given
each new instance of a bus, the program will warmstart a KCF tracker at the given frame for that bus and manage the tracker for as
long as the bus is in the frame. 

The script will export 2 files:

1. A H.264 encoded mp4 file that has the tracked bounding boxes for each bus that was found by the tracker. The generated output
can be seen [here](data/taipei-hires-2017-04-08-annotated.mp4)
2. A json file that contains an array of objects, one per frame. Each object has a frame number associated with it along with an array
containing the boxes (of busses) found at the frame. The generated json output file can be seen [here](data/taipei-hires-2017-04-08-boxes.json) 

An example frame object follows the example below:
```$xslt
{
    "frame" : 1298,             //the frame number
    "boxes" : [
      {
        "bus_index" : 0,        //which bus was seen here (index number gotten from the dataset)
        "xmin" : 401,           //Coordinates of the box
        "ymin" : 132,
        "xmax" : 732,
        "ymax" : 288
      }
    ]
  }
```
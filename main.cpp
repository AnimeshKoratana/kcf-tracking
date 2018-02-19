/*
 * @author: Animesh Koratana <koratana@stanford.edu>
 * Bus Tracking Feature with the KCF Tracker
 */
#include <opencv2/core/core.hpp>        // Basic OpenCV structures (cv::Mat)
#include <opencv2/tracking.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <fstream>      // std::ifstream
#include <queue>

#include <folly/json.h>

using namespace std;
using namespace cv;

using folly::dynamic;
using folly::toPrettyJson;

//enum cols{frame =1, object_name = 2, confidence = 3, xmin =4, ymin = 5, xmax=6, ymax = 7, index = 8};
const string video_file = "data/taipei-hires-2017-04-08.mp4";
const string video_file_out = "data/taipei-hires-2017-04-08-annotated.mp4";
const string boxes_file_out = "data/taipei-hires-2017-04-08-boxes.json";
const string dataset_file = "data/taipei-hires-2017-04-08.csv";
const bool render = false; //set this to true to render results

struct bus_frame{
    int frame_num;
    int bus_index;
    double confidence;
    Rect roi;
};

struct bus_tracker{
    Ptr<Tracker> tracker;
    int bus_index;
    Rect2d roi;
};

void initialize_bus_instances(const string dataset, queue<bus_frame>& frames);

int main(){
    // create the tracker list
    vector<struct bus_tracker> trackers;
    //Initialize the input video
    VideoCapture cap(video_file);
    if( ! cap.isOpened () ) return -1;

    //Initialize the dataset and find the first occurrence of each video
    queue<bus_frame> bus_frames;
    initialize_bus_instances(dataset_file, bus_frames);

    printf("Start the tracking process\n");
    Mat current_frame; //current frame

    //Write the tracked video
    VideoWriter outputVideo(video_file_out, CV_FOURCC('X', '2', '6', '4'), cap.get(CV_CAP_PROP_FPS),
                            Size((int)cap.get(CV_CAP_PROP_FRAME_WIDTH), (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT)));
    dynamic output_json = dynamic::array();
    for( ; ; ){
        cap >> current_frame;
        // stop the program if no more images
        if(current_frame.rows==0 || current_frame.cols==0) break;

        //Initialize a new tracker for the bus found at this frame
        if((int)cap.get(CV_CAP_PROP_POS_FRAMES) == bus_frames.front().frame_num){
            Ptr<Tracker> tracker = TrackerKCF::create();
            struct bus_tracker btracker;
            btracker.tracker = tracker;
            btracker.tracker->init(current_frame, bus_frames.front().roi); //warm start the tracker at this frame
            btracker.roi = bus_frames.front().roi;
            btracker.bus_index = bus_frames.front().bus_index;
            trackers.push_back(btracker); //add it to the list of current trackers
            bus_frames.pop(); //we started tracking this bus, so go to the next one
        }

        //update each tracker
        dynamic frame_boxes = dynamic::array(); //will contain all of the tracked busses we see in this frame
        for(int i =0; i<trackers.size(); i++){
            struct bus_tracker btracker = trackers[i];
            bool found = btracker.tracker->update(current_frame, btracker.roi);
            if(found){
                rectangle(current_frame, btracker.roi, Scalar( 255, 0, 0 ), 2, 1 );
                dynamic fbox = dynamic::object
                                           ("bus_index", btracker.bus_index)
                                           ("xmin", btracker.roi.x)
                                           ("ymin", btracker.roi.y)
                                           ("xmax", btracker.roi.x + btracker.roi.width)
                                           ("ymax", btracker.roi.y + btracker.roi.height);
                frame_boxes.push_back(fbox);
            }else{
                trackers.erase(trackers.begin() + i); //this bus is out of the frame, so remove it from the tracked busses
            }
        }
        if(!frame_boxes.empty()){ //if we found a bus
            dynamic frame_object = dynamic::object
                    ("frame", (int)cap.get(CV_CAP_PROP_POS_FRAMES))
                    ("boxes", frame_boxes);
            if (render) cout << toJson(frame_object) << endl;
            output_json.push_back(frame_object);
        }

        // store image with the tracked object
        outputVideo << current_frame;

        if (render){
            imshow("Bus tracker",current_frame);
            if(waitKey(1)==27)break;
        }
    }

    //Output the JSON File
    printf("Writing JSON File...\n");
    ofstream outdata; //output for json boxes
    outdata.open(boxes_file_out);
    if(!outdata) return -1;
    outdata << toPrettyJson(output_json);
    outdata.close();
    outputVideo.release();
    return 0;
}

void initialize_bus_instances(const string dataset, queue<bus_frame>& frames){
    ifstream file (dataset);
    string line;
    int bus_index = -1;
    getline(file, line);
    while(getline(file, line)){
        stringstream line_stream(line);
        string cell;
        struct bus_frame bframe;
        string obj_name;

        //Col 1: frame
        getline(line_stream, cell, ',');
        bframe.frame_num = (int)atof(cell.c_str());

        //Col 2: Object Name
        getline(line_stream, obj_name, ',');

        if(obj_name == "bus"){ //can switch to car to see that
            //Col 3: Confidence
            getline(line_stream, cell, ',');
            bframe.confidence = atof(cell.c_str());

            //Col 4: xmin
            getline(line_stream, cell, ',');
            double xmin = atof(cell.c_str());
            //Ymin
            getline(line_stream, cell, ',');
            double ymin = atof(cell.c_str());
            //xmax
            getline(line_stream, cell, ',');
            double xmax = atof(cell.c_str());
            //ymax
            getline(line_stream, cell, ',');
            double ymax = atof(cell.c_str());
            //index
            getline(line_stream, cell, ',');
            int index = (int)atof(cell.c_str());

            if(index > bus_index){
                Rect roi_box(xmin, ymin, xmax-xmin, ymax-ymin);
                bframe.bus_index = index;
                bframe.roi = roi_box;
                frames.push(bframe);
                bus_index = index;
            }
        }
    }
}
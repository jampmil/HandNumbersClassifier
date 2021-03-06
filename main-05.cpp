/**
    Hand Gesture Classifier program, based on the OpenCV library and using a SVM model.
*/

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ml/ml.hpp>
#include "Clock.h"
#include "dirent.h"
#include <string>
#include <vector>
#include <fstream>


using namespace std;
using namespace cv;
using namespace cv::ml;


/**
    Function that trims from the start a string in place
    Params:
        s - The string to trim
*/
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
}

/**
    Function that reads the HSV config file (hsv.config).
    Returns: an array of integers with the values of the HSV configuration in the format: {minH, maxH, minS, maxS, minV, maxV}
*/
int* readHsvConfigFile(){

    int hsvConfig [6];
    bool badReading = false;
    string line;
    ifstream myfile ("hsv.config");
    if (myfile.is_open()) {
        while ( getline (myfile,line) )
        {
            ltrim(line);
            if(line.at(0) != '#' && line.c_str() != ""){
                //cout << line << '\n';

                stringstream splitted (line);
                int i = 0;
                string strNum;
                while(splitted >> strNum){
                    hsvConfig[i] = atoi( strNum.c_str() );
                    i++;
                }

                if(i != 6){
                    badReading = true;
                }
            }
        }
        myfile.close();
    }
    else
        badReading = true;

    if (badReading)
        return NULL;
    else
        return hsvConfig;
}

/**
    Function that writes the HSV configuration in its file (hsv.config)
    Params:
        hsvConfig - an array of integers with the values of the HSV configuration in the format: {minH, maxH, minS, maxS, minV, maxV}
*/
void writeHsvConfig(int * hsvConfig){

    int minH = hsvConfig[0], maxH = hsvConfig[1], minS = hsvConfig[2], maxS = hsvConfig[3], minV = hsvConfig[4], maxV = hsvConfig[5];
    ofstream myfile ("hsv.config");
    if (myfile.is_open())
    {
        myfile << "#HSV Config in the format: minH maxH minS maxS minV maxV\n";
        myfile << "#130 160 10 40 75 130\n";
        myfile <<  minH << " " << maxH << " "<< minS << " " << maxS << " " << minV << " " << maxV;
        myfile.close();
    }
    else cout << "Error writing HSV Config File\n";
}

/**
    Function that process an image applying a thresholding to find the best contour of a hand in dark background
    Params:
        img - A matrix of the image to process
        hsvConfig - The HSV Configuration to apply the threshold
    Returns: A matrix of the processed image.
*/
Mat processImage(Mat img, int* hsvConfig){

    // Get the specific HSV config
    int minH = hsvConfig[0], maxH = hsvConfig[1], minS = hsvConfig[2], maxS = hsvConfig[3], minV = hsvConfig[4], maxV = hsvConfig[5];

    cv::Mat hsv;
    cv::cvtColor(img, hsv, CV_BGR2HSV);
    cv::inRange(hsv, cv::Scalar(minH, minS, minV), cv::Scalar(maxH, maxS, maxV), hsv);
    int blurSize = 5;
    int elementSize = 5;
    cv::medianBlur(hsv, hsv, blurSize);
    cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2 * elementSize + 1, 2 * elementSize + 1), cv::Point(elementSize, elementSize));
    cv::dilate(hsv, hsv, element);

    // Resize image
    Size sizeImg(640, 480);
    cv::resize(hsv, hsv, sizeImg);

    return hsv;

}

/**
    Function to configure the camera in order to find the best HSV configuration for the thresholding process for the images.
    It reads the current configuration from the hsv.config file and when the user is done configuring it writes the new configuration into the file.
*/
void configureCamera(){

    // Print the instructions
    cout << "Use the sliders to find the best configuration possible. Press any key when you are done..." << endl;

    // Launch the camera
    cv::VideoCapture cap(0);

    // Read the values in the config file
    //int minH = 130, maxH = 160, minS = 10, maxS = 40, minV = 75, maxV = 130;
    int* hsvConfig = readHsvConfigFile();
    int minH = hsvConfig[0], maxH = hsvConfig[1], minS = hsvConfig[2], maxS = hsvConfig[3], minV = hsvConfig[4], maxV = hsvConfig[5];

    // Create the window to show the help image
    const char* windowHelp = "Hand Gesture Prediction";
    namedWindow(windowHelp);
    Mat imgHelp = imread("support_images//config_help.png");
    imshow(windowHelp, imgHelp);
    moveWindow(windowHelp, 550, 0);

    // Create the window to show the camera input
    const char* windowCamera = "Configure Camera";
    cv::namedWindow(windowCamera);
    imshow(windowCamera, imgHelp);
    moveWindow(windowCamera, 1000, 0);
    cv::createTrackbar("MinH", windowCamera, &minH, 180);
    cv::createTrackbar("MaxH", windowCamera, &maxH, 180);
    cv::createTrackbar("MinS", windowCamera, &minS, 255);
    cv::createTrackbar("MaxS", windowCamera, &maxS, 255);
    cv::createTrackbar("MinV", windowCamera, &minV, 255);
    cv::createTrackbar("MaxV", windowCamera, &maxV, 255);

    while (1)
    {
        // Capture the frame
        cv::Mat frame;
        cap >> frame;

        //Process the image
        int hsvConfigCurrent [6] = {minH, maxH, minS, maxS, minV, maxV};
        Mat hsv = processImage(frame, hsvConfigCurrent);

        //Show the image in the screen
        cv::imshow(windowCamera, hsv);

        // Print current config
        //cout << "int minH = " << minH << ", maxH = " << maxH << ", minS = "<< minS << ", maxS = " << maxS << ", minV = " << minV << ", maxV = " <<maxV << ";" << endl;

        // If a key is pressed exit the loop
        if (cv::waitKey(30) >= 0) break;
    }

    // Close the window when finished
    cvDestroyWindow(windowCamera);
    cvDestroyWindow(windowHelp);

    // Write the final values into the config file
    int hsvConfigCurrentFinal [6] = {minH, maxH, minS, maxS, minV, maxV};
    writeHsvConfig(hsvConfigCurrentFinal);
    //#HSV Config in the format: minH maxH minS maxS minV maxV
    //130 160 10 40 75 130

}

/**
    Function that loops over the images folder, imports and process the images and divides them into the training and testing sets (~30% for testing)
    Params:
        trainData - A matrix with the images (one image per row) of the training set
        trainClasses - A matrix of one column and the classes of the training set (according to the trainData images)
        testData - A matrix with the images (one image per row) of the testing set
        testClasses - A matrix of one column and the classes of the testing set (according to the testData images)
        trainingInData - An object with the training data ready to be used for training a model
*/
void createData( Mat& trainData, Mat& trainClasses, Mat&testData, Mat& testClasses, Ptr<TrainData>& trainingInData)
{
    // Indicates if it should show the images that are being processed or not
    bool showTraining = false;

    // Loop over the images folder to load and processed the images for the dataset
    DIR *dir;
    struct dirent *ent;
    DIR *dirClass;
    struct dirent *imgFile;
    if ((dir = opendir ("images\\")) != NULL) {
      // This is the images folder... now we loop through all files and if it is a dir then we go inside
      printf ("Dir: %s\n", dir->dd_name);
      while ((ent = readdir (dir)) != NULL) {
        // Here we are in the files of the images folders
        if (strcmp(ent->d_name, ".") != 0 & strcmp(ent->d_name, "..") != 0){

            // Path of the file/folder in the images folder
            stringstream ss;
            ss << "images\\" << ent->d_name;
            string ss2 = ss.str();
            const char * dirClassPath = ss2.c_str();

            // Static values for perfect threshold of training test images
            //int minH = 10, maxH = 160, minS = 10, maxS = 200, minV = 10, maxV = 130;
            int minH = 10, maxH = 160, minS = 0, maxS = 200, minV = 10, maxV = 130;

            //For showing only
            const char* windowName = "Training Hand Gesture Classifier";
            if(showTraining){
                namedWindow(windowName);
                cv::createTrackbar("MinH", windowName, &minH, 180);
                cv::createTrackbar("MaxH", windowName, &maxH, 180);
                cv::createTrackbar("MinS", windowName, &minS, 255);
                cv::createTrackbar("MaxS", windowName, &maxS, 255);
                cv::createTrackbar("MinV", windowName, &minV, 255);
                cv::createTrackbar("MaxV", windowName, &maxV, 255);
            }


            // if it's a folder then go inside and loop for files
            if ((dirClass = opendir (dirClassPath)) != NULL) {
                char* classImg = ent->d_name; //This is the class of the image
                int classImgInt = atoi(classImg);

                // Load the images and divide them between testing and training sets
                int countTest = 1;
                while ((imgFile = readdir (dirClass)) != NULL) {

                    // check if the file extension is .jpg or png
                    bool isImg = imgFile->d_namlen >= 4 && strcmp(imgFile->d_name + imgFile->d_namlen - 4, ".jpg") == 0;
                    isImg = isImg || imgFile->d_namlen >= 4 && strcmp(imgFile->d_name + imgFile->d_namlen - 4, ".png") == 0;
                    if (isImg){
                        // Here we are in the files of the images folders
                        //cout<< "Class: "<<classImgInt<<"\n";
                        //printf ("     : %s\n", imgFile->d_name);

                        stringstream sf;
                        sf << "images\\" << ent->d_name << "\\" << imgFile->d_name;
                        string sf2 = sf.str();
                        const char * dirImgPath = sf2.c_str();

                        // Load and preprocess the image
                        Mat img = imread(dirImgPath);

                        //Process the image
                        int hsvConfig [6]= {minH, maxH, minS, maxS, minV, maxV};
                        Mat hsv = processImage(img, hsvConfig);

                        // Show the image if necessary
                        if(showTraining){
                            cv::imshow(windowName, hsv);
                        }

                        // Convert to format for training
                        Mat floatImg;
                        hsv.convertTo(floatImg, CV_32F);

                        // Divide in training and testing sets (takes number 3, 5, and 8 of each 10 images ~30%)
                        // For cross validation take for test 1,2,3, then 4,5,6 and finally 7,8,9,10. Run it 3 times.
                        if(countTest == 3 || countTest == 5 || countTest == 8){
                            // ADD to testset
                            //testData.push_back(floatImg.reshape(1,1) );
                            testData.push_back(floatImg.reshape(1,1) );
                            testClasses.push_back(classImgInt);
                        } else{
                            // ADD to train
                            //trainData.push_back(floatImg.reshape(1,1) );
                            trainData.push_back(floatImg.reshape(1,1) );
                            trainClasses.push_back(classImgInt);
                        }

                        // Reset the counter if we got to 10
                        if(countTest >= 10)
                            countTest = 1;
                        else
                            countTest++;

                        // Make the reading slower if showing the Training
                        if(showTraining){
                            if (cv::waitKey(2) >= 0) break;
                        }
                    }
                }
            }

            // If the training was showed, close the window
            if(showTraining){
                cvDestroyWindow(windowName);
            }
        }

      }
      closedir (dir);
    } else {
      /* could not open directory */
      perror ("");
      //return EXIT_FAILURE;
    }

    // Print some final statistics
    cout<<"trainData size: " << trainData.size() << endl;
    cout<<"trainClasses size: " << trainClasses.size() << endl;
    cout<<"testData size: " << testData.size() << endl;
    cout<<"testClasses size: " << testClasses.size() << endl;

    // Create the object of the Train data so the model can use it.
    trainingInData=TrainData::create(trainData, ROW_SAMPLE, trainClasses);

}

/**
    Function that trains an SVM model and stores it in a file.
    It creates the training and testing sets using the function createData.
    The training set is used to train the SVM model and when is ready it uses the testing set to measure the accuracy of the model.
*/
void trainSVM(){


    // Read and create training and testing sets
    Mat trainData;
    Mat trainClasses;
    Mat testData;
    Mat testClasses;
    Ptr<TrainData> trainingInData;

    cout << "Reading and preprocessing training and testing images" << endl;
    createData(trainData, trainClasses, testData, testClasses, trainingInData);

    // Variable to check if the model should be trained... if false it only loads the model and predicts the testing set.
    bool activedTraining = false;

    //Create the SVM Model
    cout << "Creating SVM Model" << endl;
    cout<<"Elements in Training Set: "<< trainData.rows << endl;

    Ptr<SVM> svm;

    if (activedTraining) {
        // Set the parameters of the SVM Model
        svm = SVM::create();
        svm->setType(SVM::C_SVC);  //C_SVC
        svm->setC(2.67);
        svm->setGamma(5.383);
        svm->setKernel(SVM::LINEAR);  //SVM::LINEAR
        svm->setTermCriteria(TermCriteria(TermCriteria::MAX_ITER, (int)1e7, 1e-6));

        // Clock for measuring the time
        Clock C;
        C.start();

        // Train the SVM Model
        cout << "Starting training process" << endl;
        svm->train(trainingInData);  //In this case we use Ptr<TrainData>
        cout << "Finished training process" << endl;

        // Write the model into a file
        cout << "Writing SVM model's file" << endl;
        svm->save("HandNumbersClassifier_01.dat");
        cout << "SVM Model stored in file: HandNumbersClassifier_01.dat" << endl;

        C.end();
        cout<<"Elapsed time: " << (C.elapsedTime() / 1000) << " seconds" << endl;
    } else {
        // Load the model from the file
        svm = StatModel::load<SVM>("HandNumbersClassifier_01.dat");
    }

    // Predict the training set
    cout<<"Predicting over the Test Set." << endl;
    cout<<"Elements in Training Set: "<< trainData.rows << endl;
    int correctTrain =0;
    for (int k=0; k<trainData.rows; k++)
    {
        // Predict the current image of the test set
        Mat tmp = trainData.row(k)+0;   //+0 explanation see row() operator in OpenCV manual
        Mat out1;
        float response = svm->predict(tmp);

        // Print the result of the current prediction
        cout<<"TrainSet[" << k << "] - True: " << int(trainClasses.at<int32_t>(0,k)) << " | Predicted: " << response << endl;

        // Increment number of corrects if correct
        if ( (int32_t)(response) ==  trainClasses.at<int32_t>(0,k))
            correctTrain++;

    }

    // Predict the test set
    cout<<"Predicting over the Test Set." << endl;
    cout<<"Elements in Test Set: "<< testData.rows << endl;
    int correctTest =0;
    for (int k=0; k<testData.rows; k++)
    {
        // Predict the current image of the test set
        Mat tmp = testData.row(k)+0;   //+0 explanation see row() operator in OpenCV manual
        Mat out1;
        float response = svm->predict(tmp);

        // Print the result of the current prediction
        cout<<"TestSet[" << k << "] - True: " << int(testClasses.at<int32_t>(0,k)) << " | Predicted: " << response << endl;

        // Increment number of corrects if correct
        if ( (int32_t)(response) ==  testClasses.at<int32_t>(0,k))
            correctTest++;

    }

    // Print final statistics
    cout << " Train Set Prediction" << endl;
    cout << " Number of correct matches: " << correctTrain << endl;
    cout << " Accuracy: " << (correctTrain * 100.0 / double(trainData.rows)) << endl;

    cout << " Test Set Prediction" << endl;
    cout << " Number of correct matches: " << correctTest << endl;
    cout << " Accuracy: " << (correctTest * 100.0 / double(testData.rows)) << endl;

}

/**
    Function that starts the camera and predicts the class of the current input of the camera using the SVM model.
    It reads the SVN Configuration from the config file and displays the predictions to the user, while typing them on the console.
*/
void readCameraAndPredict(){

    // Load SVM Model
    cout << "Loading SVM Model" << endl;
    Ptr<SVM> svm;
    svm = StatModel::load<SVM>("HandNumbersClassifier_01.dat");
    cout << "SVM Model Loaded, Launching Camera" << endl;

    // Window for showing the predictions
    const char* windowPred = "Hand Gesture Prediction";
    namedWindow(windowPred);
    Mat imgPred = imread("support_images//0.png");
    imshow(windowPred, imgPred);
    moveWindow(windowPred, 1000, 600);

    // Windows for processed and original images
    const char* windowOriginal = "Hand Numbers Classifier: Original Image";
    imshow(windowOriginal, imgPred);
    moveWindow(windowOriginal, 550, 0);

    const char* windowProcessed = "Hand Numbers Classifier: Processed Image";
    imshow(windowProcessed, imgPred);
    moveWindow(windowProcessed, 1200, 0);

    // Read the HSV configuration
    //int minH = 130, maxH = 160, minS = 10, maxS = 40, minV = 75, maxV = 130;
    int* hsvConfig = readHsvConfigFile();
    int minH = hsvConfig[0], maxH = hsvConfig[1], minS = hsvConfig[2], maxS = hsvConfig[3], minV = hsvConfig[4], maxV = hsvConfig[5];

    // Launch and start reading from camera
    cv::VideoCapture cap(0);


    //Starts the clock - after it predicts a gesture it waits 3 seconds to try to predict the next
    bool sleep = false;
    std::clock_t start = std::clock();
    double duration;

    cout << endl << "The predicted sequence of numbers are:" << endl << endl;
    while (1)
    {
        // Capture the camera frame and show the original
        cv::Mat frame;
        cap >> frame;
        cv::imshow(windowOriginal, frame);

        //Process the image
        int hsvConfig [6]= {minH, maxH, minS, maxS, minV, maxV};
        Mat hsv = processImage(frame, hsvConfig);

        //Show the processed image
        cv::imshow(windowProcessed, hsv);

        // Convert to format for training
        Mat floatImg;
        hsv.convertTo(floatImg, CV_32F);

        // Predicting the current frame
        //cout << "Predicting"<<endl;
        Mat reshaped = floatImg.reshape(1,1);
        //cout << "rows:" << reshaped.rows << " cols: "<< reshaped.cols << endl;
        float response = svm->predict(reshaped);

        duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
        if(duration >= 3){
            sleep = false;
        }

        //cout << "sleep: " << sleep << " | elapsed: " << duration << endl;
        if(!sleep){
            // Show the correspondent image depending on the prediction
            bool didPredict = true;
            if(response == 1){
                imgPred = imread("support_images//1.png");
                imshow(windowPred, imgPred);
                cout << response ;
            } else if(response == 2){
                imgPred = imread("support_images//2.png");
                imshow(windowPred, imgPred);
                cout << response ;
            } else if(response == 3){
                imgPred = imread("support_images//3.png");
                imshow(windowPred, imgPred);
                cout << response ;
            } else if(response == 4){
                imgPred = imread("support_images//4.png");
                imshow(windowPred, imgPred);
                cout << response ;
            } else if(response == 5){
                imgPred = imread("support_images//5.png");
                imshow(windowPred, imgPred);
                cout << response ;
            } else {
                imgPred = imread("support_images//0.png");
                imshow(windowPred, imgPred);
                didPredict = false;
            }

            // Restarts the inactive clock if it did predict
            if (didPredict){
                //Restarts
                start = std::clock();
                sleep = true;

            }

        }

        // If a key is pressed stops the loop and closes the windows
        if (cv::waitKey(30) >= 0) {
            cout << endl << endl << "Closing the prediction." << endl ;
            // Close the windows
            cvDestroyWindow(windowPred);
            cvDestroyWindow(windowOriginal);
            cvDestroyWindow(windowProcessed);
            break;
        }
    }
}


/**
    Main function of the program.
    It creates an interactive program so the user can select the respective options by typing them on the console.
*/
int main( int argc, char** argv )
{
    // Print the title of the program
    cout << "Hand Gesture Classifier" << endl;


    // Variable to control if the program should stop
    bool endProgram = false;

    // Cycle that indicates the program is running
    while(!endProgram){

        // Print the guide of the program.
        cout << endl;
        cout << "The following options are available. Press the correspondent key to continue:" << endl;
        cout << "P - Starts the Camera and reads the hand gestures shown to it." << endl;
        cout << "C - Configure the threshold of the camera for a better prediction." << endl;
        cout << "T - Train the SVM model based on the images present in the images folder." << endl;
        cout << "Q - Quits this program." << endl;
        cout << endl << "Select an option to continue..." << endl;

        // Get the user input to proceed with the program
        string input = "";
        getline(cin, input);
        int keyPressed = input.at(0);//getchar();
        cout << endl;

        // Check the user input
        if(keyPressed == 80 || keyPressed == 112) {
            cout << "Read from Camera and Predict" << endl;
            //Read from Camera and Predict
            readCameraAndPredict();

        } else if(keyPressed == 67 || keyPressed == 99) {
            cout << "Configure Webcam" << endl;
            // Configure Webcam
            configureCamera();

        } else if(keyPressed == 84 || keyPressed == 116) {
            cout << "Train Model" << endl;
            //Train Model
            trainSVM();
        } else if(keyPressed == 81 || keyPressed == 113) {
            cout << "Exit" << endl;
            //Exit the program.
            endProgram = true;
        } else {
            cout << "Option not valid, please try again." << endl;
        }


        //cout << "Key pressed: " << keyPressed << endl;
    }

    exit(0) ;

}

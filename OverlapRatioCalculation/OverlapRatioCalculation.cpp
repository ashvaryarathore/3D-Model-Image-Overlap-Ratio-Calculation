// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
//   Run program: Ctrl + F5 or Debug > Start Without Debugging menu
//   Debug program: F5 or Debug > Start Debugging menu

//This is for Windows only as of now.

// Overlap_Ratio_Calculation.cpp : 
//file contains the 'main' function. 
//Program execution begins and ends there.

//header files
#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <stdio.h>
#include <fstream> 
#include <xlsxwriter.h>
#include <math.h>

using namespace std; 

/************************************************************************
workbook structure:
It will hold the row, column, workbook name and worksheet informaiton
This same struct object will be passed along in the functions needing 
the workbook information.
*************************************************************************
struct WorkBook
{
    int row;
  
    lxw_workbook* workbook;
    lxw_worksheet* worksheet;

    WorkBook()
    {
        row = 0;
        workbook = workbook_new("C:\\Aishvarya\\Sample\\Overlap_Calculation.xlsx");
        worksheet = workbook_add_worksheet(workbook, "Overlap_Calculation");
    }

    void incrementRow()
    {
        row = row + 1;
    }
};

/*************************************************************************/

//global params
long double objectDistance = 0;
long double bearing = 0;
int objectType = 1; 
//1: straight path
//2: circular path
vector<long double> oldValues(8, 0);
/*values in this vector:
vector<long double> oldValues(7, 0);
setValues = 0 : default
lastFOV
lat1
long1
old bearing
average OR
average BR
count
*/

/***********************************************************************
Function: calculateInitialBearingDistance()
Function will calculate and store the bearing with object distance using
the initial locations provided by the user.
Later, the calculated bearing and obj distance will be used to find new
object cordinates. (Assuming the user will fly UAV in a straight path)
Will not work otherwise. :')
For circular paths (ideal scenario), the center will be treated as obj
location for all cases. (Hope user fly's the UAV in circular path.)
************************************************************************/
void calculateInitialBearingDistance(long double &objectDistance, long double &bearing, vector<long double> cameraPos, vector<long double> objectPos)
{
    long double lat1, lat2, long1, long2;
    const long double PI = 3.141592653589793238463;

    //convert the lat, longs to radians
    lat1 = cameraPos[0] * (PI / 180);
    long1 = cameraPos[1] * (PI / 180);
    lat2 = objectPos[0] * (PI / 180);
    long2 = objectPos[1] * (PI / 180);

    //distance is calculated using the Haversine formula
    //Haversine formula : a = sin²(Δφ / 2) + cos φ1 ⋅ cos φ2 ⋅ sin²(Δλ / 2)
    //  c = 2 ⋅ atan2(√a, √(1−a))
    //  d = R ⋅ c
    int radius = 6371; // Km
    long double a = sin((lat2 - lat1) / 2) * sin((lat2 - lat1) / 2) + cos(lat1) * cos(lat2) * sin((long2 - long1) / 2) * sin((long2 - long1) / 2);
    long double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    //cout << "\n calculated distance = " << radius * c;
    objectDistance = radius * c;
    //cout << " objectDistance = " << objectDistance;

    //bearing calculation
    long double y = sin(long2 - long1) * cos(lat2);
    long double x = cos(lat1) * sin(lat2) - (sin(lat1) * cos(lat2) * cos(long2 - long1));
    bearing = atan2(y, x);
    //cout << "\n bearing = " << bearing;
    bearing = bearing * (180 / PI);
    bearing = fmod(bearing+360, 360);
    bearing = bearing * (PI / 180);
    //cout << " \n changed bearing = " << bearing;
}

/*******************************************************************
Function: calculateObjectCordinates(vector<long double> &objectCordinates)
Will evaluate the object cordinate given bearing and
********************************************************************/
void calculateObjectCordinates(vector<long double>& objectCordinates, vector<long double> cameraCordinates, long double objectDistance, long double bearing)
{
    //using the camera cords, distance and bearing find the obj cords.
    long double lat1, lat2, long1, long2;
    const long double PI = 3.141592653589793238463;
    //cout <<std::fixed<< " \n after called : lat1: " << cameraCordinates[0] << "  long 1 : " << cameraCordinates[1];

    //convert the lat, longs to radians
    lat1 = cameraCordinates[0] * (PI / 180);
    long1 = cameraCordinates[1] * (PI / 180);
    int radius = 6371; // Km
    //cout << "\n objectDistance = " << objectDistance;
    //cout << " \n bearing = " << bearing;
    //cout << " \n lat1: " << cameraCordinates[0] << " long1: " << cameraCordinates[1];
    
    lat2 = asin(sin(lat1) * cos(objectDistance / radius) + cos(lat1) * sin(objectDistance / radius) * cos(bearing));
    long2 = long1 + atan2(sin(bearing) * sin(objectDistance / radius) * cos(lat1), cos(objectDistance / radius) - sin(lat1) * sin(lat2));

    objectCordinates[0] = (lat2);
    objectCordinates[1] = (long2);

    lat2 = lat2 * (180 / PI);
    long2 = long2 * (180 / PI);
    //cout << std::fixed <<"\n lat2: " << lat2 << " |  long 2 :" << long2;
    //cout<<std::fixed<< "\n lat2: " << lat2 * (PI / 180) << " |  long 2 :" << long2 * (PI / 180);
    
}



/************************************************************************
Function: writeToXls()
This function will write the passed parameters to xls file.
Making a generic funtion to reuse for different parameters.
*************************************************************************/
void writeToXls(lxw_worksheet* worksheet, vector<long double>& objectCordinates, vector<long double>& cameraCordinates, int& row, std::string imageName)
{
    const char* cstr = imageName.c_str();

    int column = 0;
    //cout << std::fixed << "\n lat2: " << objectCordinates[0] << " |  long 2 :" << objectCordinates[1];
    //cout << "\n ------------ ";
    //object and camera cordinates in degrees
    const long double PI = 3.141592653589793238463;

    long double objectLatDeg = objectCordinates[0] * (180 / PI);
    long double objectLongDeg = objectCordinates[1] * (180 / PI);
    worksheet_write_number(worksheet, row, column++, objectLatDeg, NULL);
    worksheet_write_number(worksheet, row, column++, objectLongDeg, NULL);
    worksheet_write_number(worksheet, row, column++, cameraCordinates[0], NULL);
    worksheet_write_number(worksheet, row, column++, cameraCordinates[1], NULL);

    //change cordinates to radian
    long double cam_lat, cam_long, obj_lat, obj_long, long1, long2, lat1, lat2;
    int radius = 6371;

    //convert coridnates to radian 
    cam_lat = cameraCordinates[0] * (PI / 180);
    cam_long = cameraCordinates[1] * (PI / 180);
    obj_lat = objectCordinates[0]; 
    obj_long = objectCordinates[1];
    
    //calculate Distance_Obj_Camera :  distance between object and camera
    //Haversine formula : a = sin²(Δφ / 2) + cos φ1 ⋅ cos φ2 ⋅ sin²(Δλ / 2)
    //  c = 2 ⋅ atan2(√a, √(1−a))
    //  d = R ⋅ c
    long double a = sin((obj_lat - cam_lat) / 2) * sin((obj_lat - cam_lat) / 2) + cos(cam_lat) * cos(obj_lat) * sin((obj_long - cam_long) / 2) * sin((obj_long - cam_long) / 2);
    long double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    long double objectDistance = radius * c;

    //bearing calculation
    //Since atan2 returns values in the range -π ... +π (that is, -180° ... +180°), 
    //to normalise the result to a compass bearing (in the range 0° ... 360°, with −ve values transformed into the range 180° ... 360°), 
    //convert to degrees and then use (θ+360) % 360, where % is (floating point) modulo.
    long double y = sin(obj_long - cam_long) * cos(obj_lat);
    long double x = cos(cam_lat) * sin(obj_lat) - (sin(cam_lat) * cos(obj_lat) * cos(obj_long - cam_long));
    long double bearing = atan2(y, x);
    bearing = bearing * (180 / PI);
    bearing = fmod(bearing + 360, 360);
    long double bearing45Plus = (bearing + 42) < 360 ? (bearing + 42) : ((bearing + 42) - 360);
    long double bearing45Minus = (bearing - 42) > 0 ? (bearing - 42) : (360 + (bearing - 42));
    bearing45Plus = bearing45Plus * (PI / 180);
    bearing45Minus = bearing45Minus * (PI / 180);

    //calculate cosFOV and the side length of the FOV triangle = side/cos. 
    long double cosFOV = cos(42 * (PI / 180));
    long double sideFOVTriangle = objectDistance / cosFOV;

    //calculating the end point cordinates for the given camera location. 
    //we will usw both the bearings (45 + and 45- to find cordinates 1 and 2 )
    //Formula:	φ2 = asin( sin φ1 ⋅ cos δ + cos φ1 ⋅ sin δ ⋅ cos θ )
    //λ2 = λ1 + atan2(sin θ ⋅ sin δ ⋅ cos φ1, cos δ − sin φ1 ⋅ sin φ2)
    //where	φ is latitude, λ is longitude, θ is the bearing(clockwise from north), δ is the angular distance d / R; d being the distance travelled, R the earth’s radius
    //bearing 45+  
    lat1 = asin(sin(cam_lat) * cos(objectDistance / radius) + cos(cam_lat) * sin(objectDistance / radius) * cos(bearing45Plus));
    long1 = cam_long + atan2(sin(bearing45Plus) * sin(objectDistance / radius) * cos(cam_lat), cos(objectDistance / radius) - sin(cam_lat) * sin(lat1));
    //bearing 45-  
    lat2 = asin(sin(cam_lat) * cos(objectDistance / radius) + cos(cam_lat) * sin(objectDistance / radius) * cos(bearing45Minus));
    long2 = cam_long + atan2(sin(bearing45Minus) * sin(objectDistance / radius) * cos(cam_lat), cos(objectDistance / radius) - sin(cam_lat) * sin(lat2));


    //calculating FOV distance for the points 
    //Haversine formula : a = sin²(Δφ / 2) + cos φ1 ⋅ cos φ2 ⋅ sin²(Δλ / 2)
    //  c = 2 ⋅ atan2(√a, √(1−a))
    //  d = R ⋅ c
    a = sin((lat1 - lat2) / 2) * sin((lat1 - lat2) / 2) + cos(lat2) * cos(lat1) * sin((long1 - long2) / 2) * sin((long1 - long2) / 2);
    c = 2 * atan2(sqrt(a), sqrt(1 - a));
    long double FOVDistance = radius * c;


    long double lat1Deg = lat1 * (180 / PI);
    long double long1Deg = long1 * (180 / PI);
    long double lat2Deg = lat2 * (180 / PI);
    long double long2Deg = long2 * (180 / PI);

    worksheet_write_number(worksheet, row, column++, cam_lat, NULL);
    worksheet_write_number(worksheet, row, column++, cam_long, NULL);
    worksheet_write_number(worksheet, row, column++, objectDistance, NULL);
    worksheet_write_number(worksheet, row, column++, bearing, NULL);
    worksheet_write_number(worksheet, row, column++, bearing45Plus, NULL);
    worksheet_write_number(worksheet, row, column++, bearing45Minus, NULL);
    worksheet_write_number(worksheet, row, column++, cosFOV, NULL);
    worksheet_write_number(worksheet, row, column++, sideFOVTriangle, NULL);
    worksheet_write_number(worksheet, row, column++, radius, NULL);
    worksheet_write_number(worksheet, row, column++, lat1Deg, NULL);
    worksheet_write_number(worksheet, row, column++, long1Deg, NULL);
    worksheet_write_number(worksheet, row, column++, lat2Deg, NULL);
    worksheet_write_number(worksheet, row, column++, long2Deg, NULL);
    worksheet_write_number(worksheet, row, column++, FOVDistance, NULL);

    /*values in this vector:
    vector<long double> oldValues(7, 0);
    [0] setValues = 0 : default
    [1] lastFOV
    [2] lat1
    [3] long1
    [4] old bearing
    [5] average OR
    [6] average BR
    [7] count
    */

    if (oldValues[0] == 0)
    {
        oldValues[1] = FOVDistance;
        oldValues[2] = lat1;
        oldValues[3] = long1;
        oldValues[4] = bearing;
        oldValues[0] = 1;
    }

    if ((row % 2) == 0)
    {
        long double FOVaverage = (FOVDistance + oldValues[1]) / 2;
        worksheet_write_number(worksheet, row, column++, FOVaverage, NULL);
        long double bearingDifference = abs(bearing - oldValues[4]);
        //cout << " \n\n bearing now = "<< bearing <<",  old = "<< oldValues[4]  <<"difference = " << bearing - oldValues[4];
        //check to see if the bearing difference is greater than 300
        if (bearingDifference >= 300)
        {
            //we need to modify the difference
            bearingDifference = 360 - bearingDifference;
        }
        else if (bearingDifference < 0.01)
        {
            //cout << "true";
            bearingDifference = 0;
        }
        worksheet_write_number(worksheet, row, column++, bearingDifference, NULL);

        //overalp ratio calcuation. 
        //calculating FOV distance for the points 
        //Haversine formula : a = sin²(Δφ / 2) + cos φ1 ⋅ cos φ2 ⋅ sin²(Δλ / 2)
        //  c = 2 ⋅ atan2(√a, √(1−a))
        //  d = R ⋅ c
        a = sin((oldValues[2] - lat2) / 2) * sin((oldValues[2] - lat2) / 2) + cos(lat2) * cos(oldValues[2]) * sin((oldValues[3] - long2) / 2) * sin((oldValues[3] - long2) / 2);
        c = 2 * atan2(sqrt(a), sqrt(1 - a));
        long double ORDistance = radius * c;
        cout << "\n distance or: " << ORDistance;
        cout << " \n calculatin OR between points: lat1: "<<lat2 * (180/PI)<<" LONG 1 = "<< long2 * (180 / PI);
        cout << " \n calculatin OR between points: lat2: " << oldValues[2] * (180 / PI) << " LONG 2 = " << oldValues[3] * (180 / PI);
        worksheet_write_number(worksheet, row, column++, ORDistance, NULL);
        long double ORRatio = ORDistance / FOVaverage;
        worksheet_write_number(worksheet, row, column++, ORRatio, NULL);
        oldValues[5] += ORRatio;
        oldValues[6] += bearingDifference;
        oldValues[7]++;
        oldValues[0] = 0;
    }
    else
    {
        worksheet_write_number(worksheet, row, column++, 0, NULL);
        worksheet_write_number(worksheet, row, column++, 0, NULL);
        worksheet_write_number(worksheet, row, column++, 0, NULL);
        worksheet_write_number(worksheet, row, column++, 0, NULL);

    }

    worksheet_write_string(worksheet, row, column++, cstr, NULL);
        
    //increment the row counter. 
    row = row + 1;
}


/************************************************************************
Function:parseExifToCsv
This function will get the camera location from exif file and parse it to
get the camera locations. It will not perform extraction on meta data.
The input file is assumed to be in exif format.
*************************************************************************/
void parseExifToXml(string fileName, lxw_worksheet* worksheet, vector<long double> &objectCordinates, int& row, vector<long double> &previousCameraCordinates, string &previousImage, vector<long double>& previousObjectCordinates)
{
    //fstream object
    fstream file;
    string latitude = "latitude";
    string longitude = "longitude";
    string word, previousWord = "hello";
    
    string imageName = fileName;

    if (fileName.length() >= 32)
    {
        imageName = fileName.substr(20, 25);
    }
   
  
    if (((row % 2) != 0) && (previousCameraCordinates.size() > 0))
    {
        writeToXls(worksheet, previousObjectCordinates, previousCameraCordinates, row, previousImage);
    }
   
    // opening file.
    file.open(fileName.c_str());

    //check if the file is open.
    if (file.is_open())
    {
        vector<long double> cameraCordinates;

        // extracting words from the file 
        while (file >> word)
        {
            //we need to find the latitude value.
            size_t found = previousWord.find(latitude);

            // displaying content 
            if (found != string::npos)
            {
                std::size_t current, previous = 0;
                current = word.find(',');
                if (current != std::string::npos)
                {
                    word = (word.substr(previous, current - previous));
                }

                long double cameraLat = std::stod(word);
                cameraCordinates.push_back(cameraLat);
            }
            else
            {
                found = previousWord.find(longitude);
                if (found != string::npos)
                {
                     long double cameraLong = std::stod(word);
                     cameraCordinates.push_back(cameraLong);

                     //find the object cords in case of straight path 
                     if (objectType == 1)
                     {
                         //cout <<std::fixed<< " \n before called : lat1: " << cameraCordinates[0] << "  long 1 : " << cameraCordinates[1];
                         calculateObjectCordinates(objectCordinates, cameraCordinates, objectDistance, bearing);
                     }
                     if ((row % 2) == 0)
                     {
                         previousCameraCordinates = cameraCordinates;
                         previousObjectCordinates = objectCordinates;
                         previousImage = imageName;
                     }
                     //
                     writeToXls(worksheet, objectCordinates, cameraCordinates, row, imageName);
                     //break the loop, work done.
                     break;
                }
            }
            //update the previous word
            previousWord = word;
        }
    }  
}


/************************************************************************
Function: readDirectory()
This function will read the directory and parse all the subdirectories to
get the list of files for each folder path
It will parse all the different exif data available (different images).
*************************************************************************/
void readDirectory(string dirName, vector<string>& listOfFiles)
{
    //append end character
    string dirNameTemp(dirName);
    dirNameTemp.append("\\*");

    WIN32_FIND_DATAA data;
    HANDLE hFind;
    if ((hFind = FindFirstFileA(dirNameTemp.c_str(), &data)) != INVALID_HANDLE_VALUE) 
    {
        do
        {
            listOfFiles.push_back(dirName+"\\"+data.cFileName);
            cout << "\n FileName: " << dirName + "\\" + data.cFileName;
        } 
        while (FindNextFileA(hFind, &data) != 0);
        FindClose(hFind);
    }
}


/****************************************************************************
Function: addColumNames()
Will add the required column names for calculation based on the case
****************************************************************************/
void addColumnName(lxw_worksheet* worksheet, int row, int column) 
{
    //populate the column names.
    worksheet_write_string(worksheet, row, column++, "Object_Lat", NULL);
    worksheet_write_string(worksheet, row, column++, "Object_Long", NULL);
    worksheet_write_string(worksheet, row, column++, "Camera_Lat_degrees", NULL);
    worksheet_write_string(worksheet, row, column++, "Camera_Long_degrees", NULL);
    worksheet_write_string(worksheet, row, column++, "Camera_Lat_radians", NULL);
    worksheet_write_string(worksheet, row, column++, "Camera_Long_radians", NULL);
    worksheet_write_string(worksheet, row, column++, "Distance_Obj_Camera", NULL);
    worksheet_write_string(worksheet, row, column++, "Bearing_C_O_Degrees", NULL);
    worksheet_write_string(worksheet, row, column++, "Bearing_C_O_Radian_45_Plus", NULL);
    worksheet_write_string(worksheet, row, column++, "Bearing_C_O_Radian_45_Minus", NULL);
    worksheet_write_string(worksheet, row, column++, "COS(42)", NULL);
    worksheet_write_string(worksheet, row, column++, "Fanning_Distance_Side", NULL);
    worksheet_write_string(worksheet, row, column++, "Radius_Of_Earth(m)", NULL);
    worksheet_write_string(worksheet, row, column++, "Lat1_Rad", NULL);
    worksheet_write_string(worksheet, row, column++, "Long1_Rad", NULL);
    worksheet_write_string(worksheet, row, column++, "Lat2_Rad", NULL);
    worksheet_write_string(worksheet, row, column++, "Long2_Rad", NULL);
    worksheet_write_string(worksheet, row, column++, "Fanning_Distance", NULL);
    worksheet_write_string(worksheet, row, column++, "FD_Average", NULL);
    worksheet_write_string(worksheet, row, column++, "Bearing Difference", NULL);
    worksheet_write_string(worksheet, row, column++, "Overlap_Distance", NULL);
    worksheet_write_string(worksheet, row, column++, "Overlap_Ratio", NULL);
    worksheet_write_string(worksheet, row, column++, "ImageName", NULL);
}



int main()
{
    //it worked. 
    //no compilation issues so far. :P 
    std::cout << "Hello World!\n";

    string dirName = "C:\\Aishvarya\\Sample";

    cout << "dirname = " << dirName;
    vector<string> listOfFiles;
    readDirectory(dirName, listOfFiles);

    //create a wokbook for the specific folder
    int column = 0;
    int row = 0;
    lxw_workbook* workbook = workbook_new("C:\\Aishvarya\\Sample\\Overlap_Calculation.xlsx");
    lxw_worksheet* worksheet = workbook_add_worksheet(workbook, "Overlap_Calculation");
    
    addColumnName(worksheet, row, column);
    row++;

    //vector to store the cordinates of object
    vector<long double> objectCordinates(2,0);
    vector<long double> cameraPos;
    vector<long double> objectPos;

    if(objectType == 1)
    {
        //calculate bearing and ditance using the cordinates user provides.
        objectPos.push_back(39.1387584);
        objectPos.push_back(-84.5131208);
        cameraPos.push_back(39.1387564);
        cameraPos.push_back(-84.513037);
        calculateInitialBearingDistance(objectDistance, bearing, cameraPos, objectPos);
        //house cordinates: 39.13876208, -84.51313786
    }
    else
    {
        objectCordinates[0] = (39.130962 * (3.1415926535 / 180));
        objectCordinates[1] = (-84.5133262 * (3.1415926535 / 180));
    }

    //Oscar Statue: 39.13095, -84.5134 --wrong
    //oscarStatue: 39.130962, -84.5133262
    //39.130597, -84.512752 : bearcat
    vector<long double> previousCordinates;
    vector<long double> previousObjectCordinates;
    string previousImage = " ";
    int skip = 1;
    int count = 0; 
    int step = 0;
    int parsedFiles = 0; 
    int stepVal = 2;

    //we use for each loop to iterate all the sub files
    for (int i=2; i < listOfFiles.size(); i++)
    {
        string fileName = listOfFiles[i];
        if(count < skip)
        {
            parsedFiles++;
            parseExifToXml(fileName, worksheet, objectCordinates, row, previousCordinates, previousImage, previousObjectCordinates);
            count++;
            step = stepVal;
            continue;
        }
        cout << "\n Skipping: "<<fileName;
        cout << "\n count = " << count;
        count = --step;
    }

    cout << "\n \n \n ************* parsed files = " << parsedFiles;
    worksheet_write_string(worksheet, row, column++, "Avg_OR", NULL);
    long double ORAverage = oldValues[5] / oldValues[7];
    worksheet_write_number(worksheet, row, column++, ORAverage, NULL);
    worksheet_write_string(worksheet, row, column++, "Bearing_Difference_Average", NULL);
    long double BearingDifferenceAverage = oldValues[6] / oldValues[7];
    worksheet_write_number(worksheet, row, column++, BearingDifferenceAverage, NULL);

    cout << "\n \n \n ************* \n Avg_OR = " << ORAverage;
    cout << "\n \n \n ************* \n BearingDifferenceAverage = " << BearingDifferenceAverage;

    workbook_close(workbook);
    getchar();
}

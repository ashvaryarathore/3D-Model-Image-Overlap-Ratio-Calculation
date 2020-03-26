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
    cout << "\n bearing = " << bearing;
    bearing = bearing * (180 / PI);
    bearing = fmod(bearing+360, 360);
    bearing = bearing * (PI / 180);
    cout << " \n changed bearing = " << bearing;
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
    cout << "\n objectDistance = " << objectDistance;
    cout << " \n bearing = " << bearing;
    cout << " \n lat1: " << cameraCordinates[0] << " long1: " << cameraCordinates[1];
    
    lat2 = asin(sin(lat1) * cos(objectDistance / radius) + cos(lat1) * sin(objectDistance / radius) * cos(bearing));
    long2 = long1 + atan2(sin(bearing) * sin(objectDistance / radius) * cos(lat1), cos(objectDistance / radius) - sin(lat1) * sin(lat2));

    objectCordinates[0] = (lat2);
    objectCordinates[1] = (long2);

    lat2 = lat2 * (180 / PI);
    long2 = long2 * (180 / PI);
    cout << std::fixed <<"\n lat2: " << lat2 << " |  long 2 :" << long2;
    cout<<std::fixed<< "\n lat2: " << lat2 * (PI / 180) << " |  long 2 :" << long2 * (PI / 180);
    
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
    cout << std::fixed << "\n lat2: " << objectCordinates[0] << " |  long 2 :" << objectCordinates[1];
    cout << "\n ------------ ";
    worksheet_write_number(worksheet, row, column++, objectCordinates[0], NULL);
    worksheet_write_number(worksheet, row, column++, objectCordinates[1], NULL);
    worksheet_write_number(worksheet, row, column++, cameraCordinates[0], NULL);
    worksheet_write_number(worksheet, row, column++, cameraCordinates[1], NULL);
    worksheet_write_formula(worksheet, row, column++, "= C2 * (PI()/ 180)", NULL);
    worksheet_write_formula(worksheet, row, column++, "= D2 * (PI()/ 180)", NULL);
    worksheet_write_formula(worksheet, row, column++, "= ACOS( SIN(A2)*SIN(E2) + COS(A2)*COS(E2)*COS(F2-B2) ) * 6371", NULL);
    worksheet_write_formula(worksheet, row, column++, "= ATAN2(COS(E2) * SIN(A2) - SIN(E2) * COS(A2) * COS(B2 - F2),SIN(B2 - F2) * COS(A2))", NULL); 
    worksheet_write_formula(worksheet, row, column++, "= H2*(180/PI())", NULL); 
    worksheet_write_formula(worksheet, row, column++, "= MOD(I2+360,360)", NULL); 
    worksheet_write_formula(worksheet, row, column++, "= IF((J2 + 42) < 360, J2 + 42, (J2 + 42) - 360)", NULL);
    worksheet_write_formula(worksheet, row, column++, "= IF((J2 - 42) > 0, J2 - 42, 360 + (J2 - 42))", NULL);
    worksheet_write_formula(worksheet, row, column++, "= K2 * (PI()/ 180)", NULL); 
    worksheet_write_formula(worksheet, row, column++, "= L2 * (PI()/ 180)", NULL); 
    worksheet_write_formula(worksheet, row, column++, "= COS(0.733038)", NULL);
    worksheet_write_formula(worksheet, row, column++, "= G2 / O2", NULL); 
    worksheet_write_number(worksheet, row, column++, 6371, NULL);
    worksheet_write_formula(worksheet, row, column++, "= ASIN(SIN(E2) * COS(P2 / Q2) + COS(E2) * SIN(P2 / Q2) * COS(M2))", NULL);
    worksheet_write_formula(worksheet, row, column++, "= F2 + ATAN2(COS(P2 / Q2) - SIN(E2) * SIN(R2), SIN(M2) * SIN(P2 / Q2) * COS(E2))", NULL);
    worksheet_write_formula(worksheet, row, column++, "= ASIN(SIN(E2)*COS(P2/Q2) + COS(E2)*SIN(P2/Q2)*COS(N2))", NULL); 
    worksheet_write_formula(worksheet, row, column++, "= F2 + ATAN2(COS(P2/Q2)-SIN(E2)*SIN(T2), SIN(N2)*SIN(P2/Q2)*COS(E2))", NULL); 
    //=ACOS(SIN(T2) * SIN(R3) + COS(T2) * COS(R3) * COS(S3 - U2)) * 6371
    //worksheet_write_formula(worksheet, row, column++, "= ACOS(SIN(R2) * SIN(T2) + COS(R2) * COS(T2) * COS(U2 - S2)) * 6371", NULL);
    worksheet_write_formula(worksheet, row, column++, "=ACOS(SIN(T2) * SIN(R3) + COS(T2) * COS(R3) * COS(S3 - U2)) * 6371", NULL);

    if ((row % 2) == 0)
    {
        worksheet_write_formula(worksheet, row, column++, "=(V2+V3)/2", NULL);
        worksheet_write_formula(worksheet, row, column++, "= ACOS(SIN(R2) * SIN(T3) + COS(R2) * COS(T3) * COS(U3 - S2)) * 6371", NULL);
        worksheet_write_formula(worksheet, row, column++, "= X3 / W3", NULL);
    }
    else
    {
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
    worksheet_write_string(worksheet, row, column++, "Bearing_C_O_Radians", NULL);
    worksheet_write_string(worksheet, row, column++, "Bearing_C_O_Degrees", NULL);
    worksheet_write_string(worksheet, row, column++, "Bearing_C_O_Degrees_Positive", NULL);
    worksheet_write_string(worksheet, row, column++, "Bearing_C_O_Degrees_45_Plus", NULL);
    worksheet_write_string(worksheet, row, column++, "Bearing_C_O_Degrees_45_Minus", NULL);
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
        objectPos.push_back(39.13876208);
        objectPos.push_back(-84.51313786);
        cameraPos.push_back(39.13876208);
        cameraPos.push_back(-84.51303786);
        calculateInitialBearingDistance(objectDistance, bearing, cameraPos, objectPos);
    }
    else
    {
        objectCordinates[0] = (39.1387837 * (3.1415926535 / 180));
        objectCordinates[1] = (-84.5131423 * (3.1415926535 / 180));
    }

    vector<long double> previousCordinates;
    vector<long double> previousObjectCordinates;
    string previousImage = " ";
    int skip = 0;

    //we use for each loop to iterate all the sub files
    for (int i=2; i < listOfFiles.size(); i++)
    {
        string fileName = listOfFiles[i];
        if(skip == 0)
        {
            parseExifToXml(fileName, worksheet, objectCordinates, row, previousCordinates, previousImage, previousObjectCordinates);
            skip = 10;
            continue;
        }
        //cout << "\n Skipping: "<<fileName;
        skip = skip - 1;
    }

    workbook_close(workbook);
    getchar();
}

#include <iostream>
#include <fstream>
#include <string>
#include <conio.h>
#include <filesystem>

using namespace std;

int main()
{
    //Start
    string folder, folderout;
    cout << "Type the folder name to use: ";
    getline(cin, folder);

    folderout = folder + ".HOG";

    //Creating output archive
    cout << "Packing HOG..." << endl;

    fstream archive;
    archive.open ( folderout, ios::binary | ios::out | ios::trunc);
    if (!archive.is_open())
    {
        cout << "Error opening file!" << endl;
        return 1;
    }

    //Writing timestamp
    cout << "Writing default timestamp..." << endl;
    unsigned int timestamp = 1074218745;

    archive.write((char*)&timestamp, sizeof(unsigned int));
    cout << "Done!" << endl;

    //Number Of Files
    cout << "Listing files..." << endl;
    int numfiles = 0;

    const filesystem::path path{ folder };
    for (const auto& dir_entry : filesystem::directory_iterator{ path })
        {
            cout << dir_entry.path() << '\n';
            numfiles++;
        }

    cout << "Number of files: " << numfiles << endl;
    archive.write((char*)&numfiles, sizeof(unsigned int));
    cout << "Done!" << endl;

    //Archive Header Length (20)
    cout << "Writing header length..." << endl;
    unsigned int header = 20;
    archive.write((char*)&header, sizeof(unsigned int));
    cout << "Done!" << endl;

    //Offset To Filename Directory
    cout << "Writing offset to filename directory..." << endl;
    int filename_offset = (numfiles*4) + 24;
    archive.write((char*)&filename_offset, sizeof(unsigned int));
    cout << "Done!" << endl;

    //Skipping some header data
    cout << "Skiping some header data (writing it later)..." << endl;
    int x = NULL;
    int offset_header = 16;
    int offset_follower = 16;

    for ( int i = 0 ; i < (numfiles+2) ; i++ )
    {
        archive.write((char*)&x, sizeof(unsigned int));
        offset_follower = offset_follower + 4;
    }
    cout << "Done!" << endl;

    //Writing filenames
    cout << "Writing filenames..." << endl;
    string current_file = " ";
    for (const auto& dir_entry : filesystem::directory_iterator{ path })
        {
            cout << dir_entry.path().filename().string() << '\n';
            archive << dir_entry.path().filename().string();
            archive.write((char*)&x, sizeof(unsigned char));
            current_file = dir_entry.path().filename().string();
            offset_follower = offset_follower + current_file.length() + 1;
        }
    if ( offset_follower % 4 != 0 )
    {
        for ( int i = 0 ; i < 4 - (offset_follower % 4); i++ )
        {
            archive.write((char*)&x, sizeof(unsigned char));
        }
    }
    cout << "Done!" << endl;

    //Writing First File Offset
    cout << "Writing First File Offset in header..." << endl;
    int first_file_offset = offset_follower + 1;
    archive.seekp(offset_header);
    archive.write((char*)&first_file_offset, sizeof(unsigned int));
    archive.seekp(offset_follower);
    offset_header = offset_header + 8;
    cout << "Done!" << endl;

    //Writing file data
    cout << "Writing file data..." << endl;
    char c;
    fstream in;
    for (const auto& dir_entry : filesystem::directory_iterator{ path })
        {
            in.open (dir_entry.path().filename().string(), ios::binary | ios::in );
            if (!in.is_open())
            {
                cout << "Error opening file!" << endl;
                return 1;
            }
            for ( int i = 1 ; i <= numfiles; i++ )
            {
                cout << i << " of " << numfiles << " are done." << endl;
            }
        }
    in.close();

    archive.close();

    getch();

    return 0;
}

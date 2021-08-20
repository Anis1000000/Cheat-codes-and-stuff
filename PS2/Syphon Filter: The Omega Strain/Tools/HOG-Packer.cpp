#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <ctime>

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

    ofstream archive;
    archive.open ( folderout, ios::binary | ios::out | ios::trunc);
    if (!archive.is_open())
    {
        cout << "Error opening file!" << endl;
        return 1;
    }

    //Writing timestamp
    cout << "Writing default timestamp..." << endl;
    time_t timestamp = time(nullptr);

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
    long int offset_follower = 16;

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
    offset_follower = offset_follower + ( 4 - (offset_follower % 4) );
    cout << "Done!" << endl;

    //Writing First File Offset
    cout << "Writing First File Offset in header..." << endl;
    int first_file_offset = offset_follower;
    archive.seekp(offset_header);
    archive.write((char*)&first_file_offset, sizeof(unsigned int));
    archive.seekp(offset_follower);
    offset_header = offset_header + 4;
    cout << "Done!" << endl;

    //Writing file data
    cout << "Writing file data..." << endl;
    char c;
    ifstream in;
    int i = 1;
    for (const auto& dir_entry : filesystem::directory_iterator{ path })
        {
            in.open (dir_entry.path(), ios::binary | ios::in );
            if (!in.is_open())
            {
                cout << "Error opening file!" << endl;
                return 1;
            }

            copy(istreambuf_iterator<char>(in), istreambuf_iterator<char>(), ostreambuf_iterator<char>(archive));

            //Writing file offset
            long int file_offset = 0;
            in.seekg(0, ios::end);
            offset_follower = offset_follower + in.tellg();
            file_offset = offset_follower + first_file_offset;
            archive.seekp(offset_header);
            archive.write((char*)&file_offset, sizeof(unsigned int));
            archive.seekp(offset_follower);
            in.close();

            offset_header = offset_header + 4;

            cout << i << " of " << numfiles << " are done." << endl;
            i = i + 1;
        }
    in.close();
    cout << "Done!" << endl;

    //Fix file offset
    cout << "Fixing one file offset..." << endl;
    archive.seekp(20);
    archive.write((char*)&x, sizeof(unsigned int));
    cout << "Done!" << endl;

    //Writing Archive Size
    cout << "Writing Archive Size..." << endl;
    long int archive_size = offset_follower + 2048;
    archive.seekp(offset_header);
    archive.write((char*)&archive_size, sizeof(unsigned int));
    cout << "Done!" << endl;
    archive.close();

    //Finish
    cout << "HOG archive packed !" << endl;

    return 0;
}

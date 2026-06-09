//
//  gerenal_processing.cpp
//  3D_measurement
//
//  Created by nozomi nishiumi on 2020/05/09.
//  Copyright © 2020 nozomi nishiumi. All rights reserved.
//

#ifndef basic_processing_hpp
#define basic_processing_hpp

#include <cmath>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <numeric>

using namespace std;

namespace basic_processing {

vector<string> split(string &input, char delimiter);

bool check_numeric(const std::string &str);

vector<vector<double>> csvloader(const string& filename);

vector<vector<string>> csvloader_str(string filename);

void csvwriter(vector<vector<vector<double>>> data,string filename);

void vector3_diff(const double *x,
                  const double *y,
                  double *result);

double euclid_norm3(const double *x);

void search_dir(string path,  vector<string> &fileNames, bool recursive=false);

string substract_subdir(string filepath, string parentdir);

string substract_name(string filepath);

string substract_oriname(string filepath, string exclude);

string substract_path(string filepath);

vector<string> scan_filepath(string dir, vector<string> name, string exclude, bool filename=true, bool recursive=true);

vector<string> scan_filepath(string dir, string name, bool filename=true, bool recursive=true);

void numeric_sort(vector<string>& files);


string getDatetimeStr();

}


#endif

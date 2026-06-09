//
//  basic_processing.cpp
//
//  Created by nozomi nishiumi on 2025/05/03.
//  Copyright © 2025 nozomi nishiumi. All rights reserved.
//

#include "basic_processing.hpp"
#include <math.h>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include <thread>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <numeric>
#include <time.h>
#include <regex>

namespace basic_processing {


vector<string> split(string &input, char delimiter) {
    istringstream stream(input);
    string field;
    vector<string> result;
    while (getline(stream, field, delimiter)) {
        result.push_back(field);
    }
    return result;
}

bool check_numeric(const std::string &str) {
    try {
        stof(str);
    }
    catch (...) {
        return false;
    }
    return true;
}

vector<vector<double>> csvloader(const string &filename) {
    ifstream ifs(filename);

    string line;
    vector<vector<double>> all;

    while (getline(ifs, line)) {
        vector<string> strvec = split(line, ',');
        vector<double> numerics;
        if (!check_numeric(strvec.at(1))) {
            continue;
        }

        for (const auto &i: strvec) {
            if (check_numeric(i)) {
                numerics.push_back(stof(i));
            } else { numerics.push_back(NAN); }
        }
        all.push_back(numerics);
    }
    return (all);
}

vector<vector<string>> csvloader_str(string filename){
    ifstream ifs(filename);

    string line;
    vector<vector<string>> all;

    while (getline(ifs, line)) {
        vector<string> strvec = split(line, ',');
        for (auto& s : strvec) {
                   s.erase(remove(s.begin(), s.end(), '\r'), s.end());
               }
        
        if (!line.empty() && line.back() == ',') {
            strvec.emplace_back();
        }
        all.push_back(strvec);
    }
    return (all);
}

void csvwriter(vector<vector<vector<double>>> data,string filename){
    ofstream ofs(filename);

    string x="_x";
    string y="_y";
    string z="_z";

    int tgt_num=(int)data.size();
    ofs <<"frameNo."<<",";
    for(int i=0;i<tgt_num;i++){
        char id[20];
        sprintf(id, "%i", i+1);
        string name_x=id+x;
        string name_y=id+y;
        string name_z=id+z;
        ofs <<
        (char*)name_x.c_str()<<","<<
        (char*)name_y.c_str()<<","<<
        (char*)name_z.c_str();

        if(i!=tgt_num-1){
            ofs<<",";
        }
    }
    ofs<<endl;



    for (int i = 0; i < data[0].size(); i++){

        ofs << i+1 << ",";
        for(int k=0;k<tgt_num;k++){


            ofs << (double)data[k][i][1] << ","
            <<(double)data[k][i][2]<<","
            <<(double)data[k][i][3];
            if(k!=tgt_num-1){
                ofs<<",";
            }

        }
        ofs << endl;
    }
}


void vector3_diff(const double *x,
                  const double *y,
                  double *result) {
    for (int i = 0; i < 3; i++) {
        result[i] = x[i] - y[i];
    }
}

double euclid_norm3(const double *x) {
    double result = 0;

    for (int i = 0; i < 3; i++) {
        result += x[i] * x[i];
    }

    return (sqrt(result));
}

void search_dir(string path,  vector<string> &fileNames, bool recursive){

    int i, dirElements;
    string search_path;
    std::string message = "";

    struct stat stat_buf;
    struct dirent **namelist=NULL;

    // dirElements にはディレクトリ内の要素数が入る
    dirElements = scandir(path.c_str(), &namelist, NULL, NULL);

    if(dirElements == -1) {
        message = "ERROR1: " + path;
    }

    else{

        //ディレクトリかファイルかを順番に識別
        for (i=0; i<dirElements; i+=1) {

            // "." と ".." を除く
            if( (strcmp(namelist[i]->d_name , ".\0") != 0) && (strcmp(namelist[i]->d_name , "..\0") != 0) ){

                //search_pathには検索対象のフルパスを格納する
                search_path = path + string(namelist[i] -> d_name);

                // ファイル情報の取得の成功
                if(stat(search_path.c_str(), &stat_buf) == 0){

                    //                     ディレクトリだった場合の処理
                    if ((stat_buf.st_mode & S_IFMT) == S_IFDIR&&recursive){
                        // 再帰によりディレクトリ内を探索
                        search_dir(path + string(namelist[i] -> d_name) + "/", fileNames,recursive);
                    }

                    //ファイルだった場合の処理
                    else {
                        size_t pos1;
                        string file_subpath;

                        pos1 = search_path.rfind('\\');
                        if(pos1 != string::npos){
                            file_subpath=search_path.substr(pos1+1, search_path.size()-pos1-1);
                        }

                        pos1 = search_path.rfind('/');
                        if(pos1 != string::npos){
                            file_subpath=search_path.substr(pos1+1, search_path.size()-pos1-1);
                        }



                        fileNames.push_back(search_path);
                    }
                }

                // ファイル情報の取得の失敗
                else{
                    message = "ERROR2: " + path;
                }
            }
        }
    }

    for (int i=0; i < dirElements; i++) {
        free(namelist[i]);
    }
    free(namelist);
    return;
}

string substract_subdir(string filepath, string parentdir){
    string dir;
    size_t pos0= filepath.size();
    size_t pos1= parentdir.size();

    if(pos0>pos1){
        dir=filepath.substr(pos1+1, pos0-pos1-1);
    }
    return dir;
}

string substract_name(string filepath){
    size_t pos1;
    pos1 = filepath.rfind('/');
    if(pos1 != string::npos){
        filepath=filepath.substr(pos1+1, filepath.size()-pos1-1);
    }
    return filepath;
}

string substract_oriname(string filepath, string exclude){
    size_t pos1;
    pos1 = filepath.rfind('/');
    if(pos1 != string::npos){
        filepath=filepath.substr(pos1+1, filepath.size()-pos1-1);
    }


    pos1 = filepath.rfind(exclude);
    if(pos1 != string::npos){
        filepath=filepath.substr(0, pos1-1);
    }

    return filepath;
}

string substract_path(string filepath){
    size_t pos1;
    pos1 = filepath.rfind('/');
    if(pos1 != string::npos){
        filepath=filepath.substr(0, pos1);
    }
    return filepath;
}

string diff_path(string filepath_all, string filepath_part){
    size_t pos1;
    pos1 = filepath_all.rfind(filepath_part);
    if(pos1 != string::npos){
        filepath_all=filepath_all.substr(pos1+filepath_part.size(),filepath_all.size()-pos1-1);
    }
    return filepath_all;
}




vector<string> scan_filepath(string dir, vector<string> name, string exclude, bool filename, bool recursive){
    vector<string> filelist0;
    vector<string> path;
    filelist0.clear();
    path.clear();

    search_dir(dir+"/", filelist0,recursive);
    sort(filelist0.begin(), filelist0.end());

    for(const auto& mainpath:filelist0){
        string tgtpath=diff_path(mainpath, dir+"/");
        bool flag_loop=false;

        auto flag_vid1=tgtpath.find(exclude);
        if(flag_vid1!=-1&&flag_vid1!=0)
            continue;

        for(int j=0; j<name.size(); j++){
            //            int flag_data=tgtpath.find(name[j]);
            //            if(flag_data==-1){
            //                flag_loop=true;
            //                continue;
            //            }

            bool flag_data2=regex_search(tgtpath.c_str(), std::regex(name[j].c_str(), regex::icase));
            if(!flag_data2){
                flag_loop=true;
                continue;
            }

        }
        if(flag_loop){
            continue;
        }




        if(filename==true){
            path.push_back(dir+"/"+tgtpath);
        }else{
            path.push_back(dir+"/"+substract_path(tgtpath));
        }
    }
    return path;
}

vector<string> scan_filepath(string dir, string name, bool filename, bool recursive){
    vector<string> filelist0;
    vector<string> path;
    filelist0.clear();
    path.clear();

    search_dir(dir+"/", filelist0,recursive);
    sort(filelist0.begin(), filelist0.end());

    for(int i=0; i<filelist0.size(); i++){
        auto flag_data=filelist0[i].find(name);
        if(flag_data==-1)
            continue;

        if(filename==true){
            path.push_back(filelist0[i]);
        }else{
            path.push_back(substract_path(filelist0[i]));

        }
    }
    return path;
}

int extractLastNumber(const std::string& s) {
    std::regex re("(\\d+)(?!.*\\d)");  // 最後に出てくる連続した数字
    std::smatch match;
    if (std::regex_search(s, match, re))
        return std::stoi(match.str());
    return 0;
}

void numeric_sort(vector<string>& files){
    std::sort(files.begin(), files.end(),
                  [](const std::string& a, const std::string& b) {
                      return extractLastNumber(a) < extractLastNumber(b);
                  });
}


string getDatetimeStr() {
    time_t t = time(nullptr);
    const tm* localTime = localtime(&t);
    std::stringstream s;
    s << "20" << localTime->tm_year - 100;
    // setw(),setfill()で0詰め
    s <<","<<setw(2) << setfill('0') << localTime->tm_mon + 1;
    s <<"," << setw(2) << setfill('0') << localTime->tm_mday;
    s <<" " << setw(2) << setfill('0') << localTime->tm_hour;
    s <<":" << setw(2) << setfill('0') << localTime->tm_min;
    s <<":" << setw(2) << setfill('0') << localTime->tm_sec;
    // std::stringにして値を返す
    return s.str();
}









}


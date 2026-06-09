#include <iostream>

// 関数宣言
int main1(int argc, char** argv);
int main2(int argc, char *argv[]);
int main3();
int main4();
int main5();
int main6(int argc, char** argv);    // 06_cam_livecalibration.cpp
int main7(int argc, char *argv[]);   // 07_cam_PnP_VR.cpp

//int main5(int argc, char** argv);

int main(int argc, char** argv) {

    int choice;

    std::cout << "Select mode (1–7): ";
    std::cin >> choice;

    switch (choice) {

        case 1:
            return main1( argc,  argv);

        case 2:
            return main2(argc,  argv);

        case 3:
            return main3( );

        case 4:
            return main4();

        case 5:
            return main5();

        case 6:
            return main6( argc,  argv);

        case 7:
            return main7(argc,  argv);   // PnP VR

        default:
            std::cout << "Invalid input. Please enter a number from 1 to 7.\n";
            return 1;
    }
}

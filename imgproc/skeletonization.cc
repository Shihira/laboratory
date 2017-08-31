// cflags: -lopencv_core -lopencv_highgui -lopencv_imgproc -fopenmp

#include <iostream>
#include <vector>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

template<typename T>
T safeAt(const Mat& inimg, int i, int j, T dflt)
{
    T gray = dflt;
    if(i > 0 && j > 0 && i < inimg.rows && j < inimg.cols)
        gray = inimg.at<T>(i, j);
    return gray;
}

void genSamplingLine(const Mat& inimg, int i, int j,
        vector<Point>& min_rel, vector<Point>& max_rel)
{
    assert(inimg.type() == CV_8U);
    const int r/*adius*/ = 10;

    float min_dvar = FLT_MAX, max_dvar;
    for(float a = 0; a < M_PI; a += M_PI / 36) {
        vector<Point> rel;
        float tan_a = tan(a); // di/dj
        if(abs(tan_a) > 1) {
            float tan_a_inv = 1 / tan_a;
            float rel_j = -r * tan_a_inv;
            for(int rel_i = -r; rel_i <= r; rel_i++, rel_j += tan_a_inv)
                rel.push_back(Point(rel_j, rel_i));
        } else {
            float rel_i = -r * tan_a;
            for(int rel_j = -r; rel_j <= r; rel_j++, rel_i += tan_a)
                rel.push_back(Point(rel_j, rel_i));
        }

        float aver = 0, dvar = 0;

        for(Point p : rel) {
            p += Point(j, i);
            uint8_t val = safeAt<uint8_t>(inimg, p.y, p.x, 255);
            aver += val;
        }
        aver /= rel.size();

        for(Point p : rel) {
            p += Point(j, i);
            uint8_t val = safeAt<uint8_t>(inimg, p.y, p.x, 255);
            dvar += (val - aver) * (val - aver);
        }
        dvar /= rel.size();

        if(dvar < min_dvar) {
            min_dvar = dvar;
            min_rel = rel;
        }

        if(dvar > max_dvar) {
            max_dvar = dvar;
            max_rel = rel;
        }
    }
}

void stripeFilter(const Mat& inimg, Mat& outimg)
{
    assert(inimg.type() == CV_8U);

    #pragma omp parallel for

    for(int i = 0; i < inimg.rows; i++)
    for(int j = 0; j < inimg.cols; j++) {
        //cout << i << ", " << j << endl;
        vector<Point> min_rel, max_rel;
        genSamplingLine(inimg, i, j, min_rel, max_rel);

        float norm_aver = 0;
        for(Point p : min_rel) {
            p += Point(j, i);
            norm_aver += inimg.at<uint8_t>(p.y, p.x);
        }
        norm_aver /= min_rel.size();

        float tang_aver = 0;
        for(Point p : max_rel) {
            p += Point(j, i);
            tang_aver += inimg.at<uint8_t>(p.y, p.x);
        }
        tang_aver /= max_rel.size();

        if(norm_aver > tang_aver)
            outimg.at<uint8_t>(i, j) = 255;
        else
            outimg.at<uint8_t>(i, j) = 0;
        
    }
}

int getbool(const Mat& inimg, int i, int j, size_t ord)
{
    assert(ord >= 0 && ord < 9);

    int8_t pos[][2] = {
        { 0, 0 }, { -1, 0 }, { -1, 1 },
        { 0, 1 }, { 1, 1 }, { 1, 0 },
        { 1, -1 }, { 0, -1 }, { -1, -1 },
    };

    int ker_i = i + pos[ord][0],
        ker_j = j + pos[ord][1];
    uint8_t gray = safeAt<uint8_t>(inimg, ker_i, ker_j, 0);

    return gray ? 1 : 0;
}

void skeletonization(const Mat& inimg, Mat& outimg)
{
    assert(inimg.type() == CV_8U);

    outimg = inimg.clone();
    Mat bufimg = outimg.clone();

    for(bool fst_iter = true, trivial; !trivial; fst_iter = !fst_iter) {
        trivial = true; // no point was removed

        for(int i = 0; i < outimg.rows; i++)
        for(int j = 0; j < outimg.cols; j++) {
            if(!getbool(outimg, i, j, 0)) continue;

            // state machine: init=1
            //  cur in      next    action
            //  0   0       0       -
            //  0   1       1       a++
            //  1   0       0       -
            //  1   1       1       -
            int state = 1, a = 0;
            for(size_t ord = 1; ord <= 9; ord++) {
                int value = getbool(outimg, i, j, ord == 9 ? 1 : ord);
                if(state == 0 && value == 1) a++;
                state = value;
            }
            if(a != 1) continue;

            int b = 0;
            for(size_t ord = 1; ord < 9; ord++) {
                b += getbool(outimg, i, j, ord);
            }
            if(b < 2 || b > 6) continue;

            int p2 = getbool(outimg, i, j, 1),
                p4 = getbool(outimg, i, j, 3),
                p6 = getbool(outimg, i, j, 5),
                p8 = getbool(outimg, i, j, 7);

            if(fst_iter) {
                if(p2 && p4 && p6) continue;
                if(p8 && p4 && p6) continue;
            } else {
                if(p2 && p4 && p8) continue;
                if(p2 && p6 && p8) continue;
            }

            trivial = false;
            bufimg.at<uint8_t>(i, j) = 0;
        }
        outimg = bufimg.clone();
    }
}

/*
void onMouse(int event, int x, int y, int, void* img_v)
{
    Mat& img = *(Mat*)img_v;
    assert(img.type() == CV_8U);

    Mat img2 = img.clone();

    if(event != EVENT_LBUTTONDOWN)
        return;

    vector<Point> min_rel, max_rel;
    genSamplingLine(img, y, x, min_rel, max_rel);
    for(Point& p : min_rel) {
        img2.at<uint8_t>(y + p.y, x + p.x) = 0;
    }

    imshow("Skeletonization", img2);
}
*/

void eliminateHoles(const Mat& inimg, Mat& outimg, int r, size_t c)
{
    for(int i = 0; i < inimg.rows; i++)
    for(int j = 0; j < inimg.cols; j++) {
        size_t sum = 0;

        for(int dy = -r; dy <= r; dy++)
        for(int dx = -r; dx <= r; dx++) {
            uint8_t val = safeAt<uint8_t>(inimg, i + dy, j + dx, 0);
            if(val < 128) sum += 1;
        }

        if(sum >= c)
            outimg.at<uint8_t>(i, j) = 0;
        else
            outimg.at<uint8_t>(i, j) = inimg.at<uint8_t>(i, j);
    }
}

int main(int argc, char** argv)
{
    namedWindow("Skeletonization");

    if(argc < 4) {
        cout << "USAGE:\n" << endl
             << "    ./skeletonization <stage_name> <inimg> <outimg>\n"
             << "STAGES:\n"
             << "    stripefilter gaussian elimhole skeleton" << endl;
        return -1;
    }

    Mat img = imread(argv[2]);
    Mat img1(img.rows, img.cols, CV_8U),
        img2(img.rows, img.cols, CV_8U);

    cvtColor(img, img1, CV_RGB2GRAY);

    int key = 0;

    do {
        if(string(argv[1]) == "stripefilter") {
            stripeFilter(img1, img2);
        }

        if(string(argv[1]) == "gaussian") {
            GaussianBlur(img1, img2, Size(5, 5), 0, 0);
        }

        if(string(argv[1]) == "elimhole") {
            eliminateHoles(img1, img2, 1, 4);
            eliminateHoles(img2, img1, 2, 12);
            eliminateHoles(img1, img2, 1, 4);
            //medianBlur(img1, img2, 3);
            //morphologyEx(img1, img2, MORPH_OPEN, Mat(2,2, CV_8U), Point(-1,-1), 1);
        }

        if(string(argv[1]) == "skeleton") {
            cv::subtract(cv::Scalar::all(255), img1, img2);
            threshold(img2, img1, 128, 255, CV_THRESH_BINARY);
            skeletonization(img1, img2);
        }
        /*
        cout << key << endl;
        if(key == 1113938)
            thers += thers == 0xff ? 0 : 1;
        if(key == 1113940)
            thers -= thers == 0x00 ? 0 : 1;
        cout << "Thershold: " << thers << endl;
        */

        imshow("Skeletonization", img2);
        imwrite(argv[3], img2);
    } while((key = waitKey(0)) > 0);
    destroyWindow("Skeletonization");
}

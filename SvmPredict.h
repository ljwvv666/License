#ifndef SVMPREDICT_H
#define SVMPREDICT_H

#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;
using namespace ml;


class SvmPredict
{
public:
	SvmPredict(const char* svm_model);
	~SvmPredict();

	//candi_plates : ��ѡ���ƣ� plate��������������ճ��ƣ�
	void doPredict(vector<Mat> candi_plates, Mat& plate);

private:
	Ptr<SVM> svm;
	HOGDescriptor* svmHog = 0;
	void getHogFeatures(HOGDescriptor* svmHog, Mat src, Mat& dst);
};


#endif // !SVMPREDICT_H

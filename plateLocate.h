#ifndef PLATELOCATE_H
#define PLATELOCATE_H

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class PlateLocate
{
public:
	PlateLocate();
	~PlateLocate();

	/**
	* src: ����⳵��ԭͼ
	* dst_plates: �����ѡ��������
	*/
	virtual void locate(Mat src, vector<Mat>& dst_plates) = 0;

protected:
	int PlateLocate::verifySizes(RotatedRect rotatedRect);
	void PlateLocate::safeRect(Mat src, RotatedRect rect, Rect2f& safa_rect);
	void PlateLocate::rotation(Mat src, Mat& dst, Size rect_size, Point2f center, double angle);
	void PlateLocate::tortuosity(Mat src, vector<RotatedRect>& rects, vector<Mat>& dst_plates);
};
#endif // !PLATELOCATE_H
